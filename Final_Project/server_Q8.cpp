// server_Q8.cpp
#include "Graph.h"
#include "Algorithms.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
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

// Utility: trim whitespace
static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

// Utility: send all bytes of a string
static void sendAll(int fd, const std::string& msg) {
    size_t sent = 0, to_send = msg.size();
    const char* data = msg.c_str();
    while (sent < to_send) {
        ssize_t n = send(fd, data + sent, to_send - sent, 0);
        if (n <= 0) break;
        sent += n;
    }
}

// Strategy interface ו־implementations (מה־Q7)…
struct AlgoStrategy {
    virtual std::string execute(const Graph& g, std::string& err) const = 0;
    virtual ~AlgoStrategy() = default;
};
struct EulerStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        if (!hasEulerianCircuit(g)) return "No Eulerian Circuit exists";
        auto circ = getEulerianCircuit(g);
        std::ostringstream o; for (int v: circ) o<<v<<" ";
        return o.str();
    }
};
struct MSTStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        return "MST weight = "+std::to_string(computeMST(g));
    }
};
struct SCCStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        auto comps = getSCCs(g);
        std::ostringstream o; o<<"SCCs:";
        for (auto& c: comps) {
            o<<" {";
            for (int v: c) o<<v<<",";
            o<<"}";
        }
        return o.str();
    }
};
struct FlowStrategy : AlgoStrategy {
    std::string execute(const Graph& g, std::string&) const override {
        int n = g.getNumVertices();
        int f = maxFlow(g, 0, n-1);
        return "MaxFlow(0->"+std::to_string(n-1)+") = "+std::to_string(f);
    }
};
struct StrategyFactory {
    std::unique_ptr<AlgoStrategy> create(const std::string& name) const {
        if (name=="euler") return std::make_unique<EulerStrategy>();
        if (name=="mst")   return std::make_unique<MSTStrategy>();
        if (name=="scc")   return std::make_unique<SCCStrategy>();
        if (name=="flow")  return std::make_unique<FlowStrategy>();
        return nullptr;
    }
};

void handleClient(int client_fd, sockaddr_in peer) {
    char peer_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    int peer_port = ntohs(peer.sin_port);
    std::cout << "[+] Client connected: " << peer_ip << ":" << peer_port << "\n";

    // Greeting + usage
    sendAll(client_fd,
            "Welcome to Graph-Server Q8 (multithreaded LF)\n"
            "Commands:\n"
            " CREATE <directed|undirected> <V> <E> (u1,v1) ... (uE,vE)\n"
            " RANDOM <directed|undirected> <V> <E> <SEED>\n"
            " QUIT\n\n"
            "Examples:\n"
            " CREATE undirected 5 3 (0,1) (1,2) (3,4)\n"
            " RANDOM directed 4 5 12345\n\n"
    );

    Graph g(0, false);
    std::string line;
    while (true) {
        // prompt
        sendAll(client_fd, "> ");

        // read a line
        char buf[2048];
        ssize_t len = recv(client_fd, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;              // client closed
        buf[len] = '\0';
        line = trim(buf);
        if (line.empty()) continue;

        std::cout << "[cmd " << peer_ip << ":" << peer_port << "] " << line << "\n";
        sendAll(client_fd, "Received: " + line + "\n");

        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd=="QUIT") {
            sendAll(client_fd,"Goodbye!\n");
            break;
        }

        // קריאה לפרמטר directed/undirected
        std::string mode;
        bool directed = false;
        if (!(iss >> mode) || (mode!="directed" && mode!="undirected")) {
            sendAll(client_fd,"ERROR: missing or invalid graph mode (directed|undirected)\n");
            continue;
        }
        directed = (mode == "directed");

        if (cmd=="CREATE") {
            int V, E;
            if (!(iss >> V >> E) || V <= 0 || E < 0) {
                sendAll(client_fd,"ERROR: invalid CREATE parameters\n");
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
                    if (u<0 || u>=V || v<0 || v>=V) { bad = true; break; }
                    g.addEdge(u, v);
                } catch (...) { bad = true; break; }
            }
            if (bad) {
                sendAll(client_fd,"ERROR: invalid edge format or out-of-range\n");
                continue;
            }
        }
        else if (cmd=="RANDOM") {
            int V, E; unsigned int seed;
            if (!(iss >> V >> E >> seed) || V <= 0 || E < 0) {
                sendAll(client_fd,"ERROR: invalid RANDOM parameters\n");
                continue;
            }
            long long maxE = 1LL * V * (V - 1) / (directed ? 1 : 2);
            if (!directed) maxE = 1LL * V * (V - 1) / 2;
            if (directed) maxE = 1LL * V * (V - 1);
            if (E > maxE) {
                sendAll(client_fd,"ERROR: too many edges for this graph type\n");
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
        else {
            sendAll(client_fd,"ERROR: unknown command\n");
            continue;
        }

        // build response: adjacency list + אלגוריתמים
        std::ostringstream resp;
        resp << "Adjacency List (" << (directed?"directed":"undirected") << "):\n";
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
        sendAll(client_fd, resp.str());
    }

    std::cout << "[-] Client disconnected: " << peer_ip << ":" << peer_port << "\n";
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
    std::cout << "[+] Server listening on port " << PORT << "\n";

    // Thread pool for leader–follower
    std::vector<std::thread> pool;
    pool.reserve(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pool.emplace_back([sock]{
            while (true) {
                sockaddr_in peer{};
                socklen_t len = sizeof(peer);
                int client = accept(sock, (sockaddr*)&peer, &len);
                if (client < 0) { perror("accept"); continue; }
                handleClient(client, peer);
            }
        });
    }
    for (auto& t: pool) t.join();
    close(sock);
    return 0;
}
