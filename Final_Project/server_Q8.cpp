#include "Graph.h"
#include "Algorithms.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <iostream>
#include <mutex>         
#include <random>
#include <set>           
#include <sstream>
#include <string>
#include <thread>
#include <vector>

constexpr int PORT = 8080;
constexpr int THREAD_COUNT = 4;

std::atomic<bool> server_running(true);  // flag for server shutdown

std::mutex clients_mutex;             
std::set<int> client_fds;            

// Utility: trim whitespace
static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// ===== Strategies =====
struct AlgoStrategy {
    virtual std::string execute(const Graph& g, std::string& err) const = 0;
    virtual ~AlgoStrategy() = default;
};
struct EulerStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        if (!hasEulerianCircuit(g)) return "No Eulerian Circuit exists";
        auto circ = getEulerianCircuit(g);
        std::ostringstream o; for (int v: circ) o << v << " ";
        return o.str();
    }
};
struct MSTStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        return "MST weight = " + std::to_string(computeMST(g));
    }
};
struct SCCStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        auto comps = getSCCs(g);
        std::ostringstream o;
        o << "SCCs:";
        for (auto& c : comps) {
            o << " {";
            for (size_t i = 0; i < c.size(); ++i) {
                o << c[i];
                if (i + 1 < c.size()) o << ",";
            }
            o << "}";
        }
        return o.str();
    }
};
struct FlowStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        int n = g.getNumVertices();
        int f = maxFlow(g, 0, n-1);
        return "MaxFlow(0->" + std::to_string(n-1) + ") = " + std::to_string(f);
    }
};
struct StrategyFactory {
    std::unique_ptr<AlgoStrategy> create(const std::string& name) const {
        if (name=="euler") return std::make_unique<EulerStrategy>();
        if (name=="mst") return std::make_unique<MSTStrategy>();
        if (name=="scc") return std::make_unique<SCCStrategy>();
        if (name=="flow") return std::make_unique<FlowStrategy>();
        return nullptr;
    }
};

bool send_block(int fd, const std::string &msg) {
    std::string full = msg + "\n===END===\n";
    const char *p = full.c_str();
    size_t rem = full.size();
    while (rem) {
        ssize_t sent = send(fd, p, rem, 0);
        if (sent <= 0) return false;
        p   += sent;
        rem -= sent;
    }
    return true;
}

void handleClient(int client_fd, sockaddr_in peer) {
    char peer_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    int peer_port = ntohs(peer.sin_port);
    std::cout << "[+] Client connected: " << peer_ip << ":" << peer_port << "\n";

    // Register client
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_fds.insert(client_fd);
    }

    send_block(client_fd,
        "Welcome to Graph-Server Q8 (multithreaded LF)\n"
        "Commands:\n"
        "  CREATE <directed|undirected> <V> <E> (u1,v1) ... (uE,vE)\n"
        "  RANDOM <directed|undirected> <V> <E> <SEED>\n"
        "  QUIT (disconnect client)\n"
        "  Q (shutdown server)\n\n"
        "Examples:\n"
        "  CREATE undirected 5 3 (0,1) (1,2) (3,4)\n"
        "  RANDOM directed 4 5 12345\n\n"
    );

    Graph g(0, false);
    std::string line;
    while (server_running) {
        char buf[2048];
        ssize_t len = recv(client_fd, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;
        buf[len] = '\0';
        line = trim(buf);
        if (line.empty()) continue;

        std::cout << "[cmd " << peer_ip << ":" << peer_port << "] " << line << "\n";
        std::ostringstream resp;
        resp << "Received: " << line << "\n";

        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "QUIT") {
            resp << "Goodbye!\n";
            send_block(client_fd, resp.str());
            std::cout << "[-] Client disconnected: " << peer_ip << ":" << peer_port << "\n";
            break;
        }

        if (cmd == "Q") {
            resp << "Server shutting down.\n";
            send_block(client_fd, resp.str());

            server_running = false;
            std::cout << "Server shutdown requested by client.\n";

            // Shutdown all other clients
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                for (int fd : client_fds) {
                    if (fd != client_fd) shutdown(fd, SHUT_RDWR);
                }
            }
            break;
        }

        if (cmd != "CREATE" && cmd != "RANDOM") {
            resp << "Invalid command. Use CREATE.. / RANDOM.. / QUIT / Q\n";
            send_block(client_fd, resp.str());
            continue;
        }

        std::string mode;
        bool directed = false;
        if (!(iss >> mode) || (mode != "directed" && mode != "undirected")) {
            resp << "Missing or invalid graph mode (directed|undirected)\n";
            send_block(client_fd, resp.str());
            continue;
        }
        directed = (mode == "directed");

        if (cmd == "CREATE") {
            int V, E;
            if (!(iss >> V >> E) || V <= 0 || E < 0) {
                resp << "Invalid CREATE parameters.\n";
                send_block(client_fd, resp.str());
                continue;
            }
            g = Graph(V, directed);
            bool bad = false;
            std::string tok;
            for (int i = 0; i < E; ++i) {
                if (!(iss >> tok) || tok.size() < 5 || tok.front() != '(' || tok.back() != ')') {
                    bad = true; break;
                }
                auto comma = tok.find(',');
                try {
                    int u = std::stoi(tok.substr(1, comma-1));
                    int v = std::stoi(tok.substr(comma+1, tok.size()-comma-2));
                    if (u < 0 || u >= V || v < 0 || v >= V) { bad = true; break; }
                    g.addEdge(u, v);
                } catch (...) { bad = true; break; }
            }
            if (bad) {
                resp << "Invalid edge format or out-of-range.\n";
                send_block(client_fd, resp.str());
                continue;
            }
        } else if (cmd == "RANDOM") {
            int V, E; unsigned int seed;
            if (!(iss >> V >> E >> seed) || V <= 0 || E < 0) {
                resp << "Invalid RANDOM parameters.\n";
                send_block(client_fd, resp.str());
                continue;
            }
            long long maxE = directed ? 1LL*V*(V-1) : 1LL*V*(V-1)/2;
            if (E > maxE) {
                resp << "Too many edges for this graph type.\n";
                send_block(client_fd, resp.str());
                continue;
            }
            g = Graph(V, directed);
            std::mt19937 gen(seed);
            std::set<std::pair<int,int>> edges;
            while ((int)edges.size() < E) {
                int a = gen() % V, b = gen() % V;
                if (!directed && a == b) continue;
                if (!directed && a > b) std::swap(a, b);
                edges.emplace(a, b);
            }
            for (auto &e : edges) g.addEdge(e.first, e.second);
        }

        resp << "Adjacency List (" << (directed ? "directed" : "undirected") << "):\n";
        auto const& adj = g.getAdjList();
        for (int i = 0; i < g.getNumVertices(); ++i) {
            resp << i << ":";
            for (int nb : adj[i]) resp << " " << nb;
            resp << "\n";
        }

        StrategyFactory fact;
        for (auto name : { "euler", "mst", "scc", "flow" }) {
            resp << "\n-- " << name << " --\n";
            std::string err;
            auto strat = fact.create(name);
            resp << (strat ? strat->execute(g, err) : "Not implemented") << "\n";
        }

        send_block(client_fd, resp.str());
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_fds.erase(client_fd);
    }

    close(client_fd);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(sock, SOMAXCONN) < 0) {
        perror("listen"); return 1;
    }
    std::cout << "Server listening on port " << PORT << "\n";

    std::vector<std::thread> pool;
    pool.reserve(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pool.emplace_back([sock]{
            while (server_running) {
                sockaddr_in peer{};
                socklen_t len = sizeof(peer);
                int client = accept(sock, (sockaddr*)&peer, &len);
                if (client < 0) {
                    if (!server_running) break;
                    perror("accept");
                    continue;
                }
                handleClient(client, peer);
            }
        });
    }

    // Waiting for shutdown
    while (server_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);

    for (auto& t: pool) t.join();
    std::cout << "Server terminated.\n";
    return 0;
}
