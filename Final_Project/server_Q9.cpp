#include "Graph.h"
#include "Algorithms.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <atomic>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cerrno>

constexpr int PORT = 8080;
std::atomic<bool> server_running(true);  // flag for server shutdown

std::mutex clients_mutex;             
std::set<int> client_fds;
std::mutex cout_mutex; 

// Utility: trim whitespace
static std::string trim(const std::string& s) {
    const char* ws = " \t\r\n";
    auto b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

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

// Thread-safe queue
template<typename T>
class SafeQueue {
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
    bool stopped = false;  
public:
    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lk(m);
            q.push(item);
        }
        cv.notify_one();
    }
    // shutdown the queue
    void stop() {
        std::lock_guard<std::mutex> lk(m);
        stopped = true;
        cv.notify_all();
    }

    T pop() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{ return !q.empty() || stopped; });
        if (q.empty()) return nullptr;
        T item = q.front();
        q.pop();
        return item;
    }
};

// RawTask: הודעה ראשונית מהלקוח (socket + מחרוזת פקודה)
struct RawTask {
    int client_fd;
    std::string request;
};

// Task: מבנה שעובר בפייפליין, עם הגרף ותשובת שרת
struct Task {
    int client_fd;
    Graph g;
    std::ostringstream resp;
    Task(int fd, Graph&& graph)
            : client_fd(fd), g(std::move(graph)) {}
};

// Global queues בין השלבים
SafeQueue<std::shared_ptr<RawTask>> q0;
SafeQueue<std::shared_ptr<Task>>    q1, q2, q3, q4, q5;

// ====================
// Stage 0 → acceptLoop
// ====================
void clientHandler(int client) {
    char peer_ip[INET_ADDRSTRLEN];
    sockaddr_in peer{};
    socklen_t len = sizeof(peer);
    getpeername(client, (sockaddr*)&peer, &len);
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    int peer_port = ntohs(peer.sin_port);
    {
        std::lock_guard<std::mutex> lk(cout_mutex);
        std::cout << "[+] Client connected: " << peer_ip << ":" << peer_port << "\n";
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_fds.insert(client);
    }

    send_block(client,
        "Welcome to Graph-Server Q9 (pipeline)\n"
        "Commands:\n"
        "  CREATE <directed|undirected> <V> <E> (u1,v1) ... (uE,vE)\n"
        "  RANDOM <directed|undirected> <V> <E> <SEED>\n"
        "  QUIT (disconnect client)\n"
        "  Q (shutdown server)\n\n"
        "Examples:\n"
        "  CREATE undirected 5 3 (0,1) (1,2) (3,4)\n"
        "  RANDOM directed 4 5 12345\n\n"
    );

    while (server_running) {
        char buf[2048];
        ssize_t rlen = recv(client, buf, sizeof(buf)-1, 0);
        if (rlen <= 0) break;
        buf[rlen] = '\0';
        auto line = trim(buf);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        if (cmd == "QUIT") {
            send_block(client, "Goodbye!\n");
            break;
        }
        if (cmd == "Q") {
            send_block(client, "Server shutting down.\n");
            server_running = false;
            break;
        }

        if (cmd != "CREATE" && cmd != "RANDOM") {
            send_block(client, "Invalid command. Use CREATE / RANDOM / QUIT / Q\n");
            continue;
        }

        auto rt = std::make_shared<RawTask>();
        rt->client_fd = client;
        rt->request   = line;
        q0.push(rt);
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_fds.erase(client);
    }
    close(client);
    {
        std::lock_guard<std::mutex> lk(cout_mutex);
        std::cout << "[-] Client disconnected: " << peer_ip << ":" << peer_port << "\n";
    }
}

void acceptLoop(int sock) {
    while (server_running) {
        sockaddr_in peer{};
        socklen_t len = sizeof(peer);
        int client = accept(sock, (sockaddr*)&peer, &len);
        if (client < 0) {
            int e = errno;
            if (!server_running || e == EBADF || e == ENOTSOCK) break;
            {
                std::lock_guard<std::mutex> lk(cout_mutex);
                perror("accept");
            }
            continue;
        }
        std::thread(clientHandler, client).detach();
    }
}

// ====================
// Stage 1: parseLoop
// ====================
void parseLoop() {
    while (true) {
        auto rt = q0.pop();
        if (!rt) break;
        int client = rt->client_fd;
        std::istringstream iss(rt->request);
        std::string cmd; iss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        Graph g(0);
        bool bad = false;
        bool directed = false;

        std::string dirstr;
        if (!(iss >> dirstr)) { send_block(client, "Missing directed/undirected\n"); continue; }
        std::string dl = dirstr;
        std::transform(dl.begin(), dl.end(), dl.begin(), ::tolower);
        if (dl == "directed") directed = true;
        else if (dl == "undirected") directed = false;
        else { send_block(client, "ERROR: invalid graph type\n"); continue; }

        if (cmd == "CREATE") {
            int V, E;
            if (!(iss >> V >> E) || V <= 0 || E < 0) { send_block(client, "Invalid CREATE parameters\n"); continue; }

            g = Graph(V, directed);
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
                } catch(...) { bad = true; break; }
            }
        } 
        else if (cmd == "RANDOM") {
            int V, E; unsigned int seed;
            if (!(iss >> V >> E >> seed) || V <= 0 || E < 0) { send_block(client, "Invalid RANDOM parameters\n"); continue; }

            long long maxE = directed ? 1LL*V*(V-1) : 1LL*V*(V-1)/2;
            if (E > maxE) { send_block(client, "Too many edges for this graph type\n"); continue; }

            g = Graph(V, directed);
            std::mt19937 gen(seed);
            std::set<std::pair<int,int>> edges;

            while ((int)edges.size() < E) {
                int a = gen() % V, b = gen() % V;
                if (!directed && a == b) continue;
                if (!directed && a > b) std::swap(a, b);
                edges.emplace(a, b);
            }

            for (auto &e : edges)
                g.addEdge(e.first, e.second);
        } else { bad = true; }

        if (bad) {
            send_block(client, "Invalid command or parameters\n");
            continue;
        }

        auto t = std::make_shared<Task>(client, std::move(g));

        // Adjacency list
        t->resp << "Adjacency List (" << (directed ? "directed" : "undirected") << "):\n";
        const auto& adj = t->g.getAdjList();
        for (int i = 0; i < t->g.getNumVertices(); ++i) {
            t->resp << i << ":";
            for (int nb : adj[i]) t->resp << " " << nb;
            t->resp << "\n";
        }

        q1.push(t);
    }
}


// ====================
// Stage 2-5 loops
// ====================
void eulerLoop() {
    while(true) {
        auto t=q1.pop();
        if (!t) break;
        std::string sec;
        if(!hasEulerianCircuit(t->g))
            sec="No Eulerian Circuit exists";
        else {
            auto circ=getEulerianCircuit(t->g);
            std::ostringstream o;
            for(int v:circ)
                o<<v<<" ";
            sec=o.str();
        }
        t->resp<<"\n-- euler --\n"<<sec<<"\n";
        q2.push(t);
    }
}

void mstLoop() {
    while(true) {
        auto t=q2.pop();
        if (!t) break;
        int w=computeMST(t->g);
        t->resp<<"\n-- mst --\nMST weight = "<<w<<"\n";
        q3.push(t);
    }
}

void sccLoop() {
    while (true) {
        auto t = q3.pop();
        if (!t) break;

        auto comps = getSCCs(t->g);
        std::ostringstream o;
        o << "SCCs:";

        for (auto& c : comps) {
            o << " {";
            bool first = true;
            for (int v : c) {
                if (!first) o << ",";
                o << v;
                first = false;
            }
            o << "}";
        }

        t->resp << "\n-- scc --\n" << o.str() << "\n";
        q4.push(t);
    }
}

void flowLoop() {
    while(true) {
        auto t=q4.pop();
        if (!t) break;
        int src=0,dst=t->g.getNumVertices()-1;
        int f=maxFlow(t->g,src,dst);
        t->resp<<"\n-- flow --\nMaxFlow("<<src<<"->"<<dst<<") = "<<f<<"\n";
        q5.push(t);
    }
}

// ====================
// Stage 6: respondLoop
// ====================
void respondLoop() {
    while (true) {
        auto t = q5.pop();
        if (!t) break;
        t->resp << "\n "; 
        send_block(t->client_fd, t->resp.str());
    }
}

// ====================
// main
// ====================

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(sock, SOMAXCONN) < 0) { perror("listen"); return 1; }

    {
        std::lock_guard<std::mutex> lk(cout_mutex);
        std::cout << "Server listening on port " << PORT << "\n";
    }

    // Start threads
    std::vector<std::thread> pool;
    pool.emplace_back(acceptLoop, sock);
    pool.emplace_back(parseLoop);
    pool.emplace_back(eulerLoop);
    pool.emplace_back(mstLoop);
    pool.emplace_back(sccLoop);
    pool.emplace_back(flowLoop);
    pool.emplace_back(respondLoop);

    // Wait for shutdown
    while (server_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Stop queues to unblock worker threads
    q0.stop();
    q1.stop();
    q2.stop();
    q3.stop();
    q4.stop();
    q5.stop();

    // Close listening socket
    shutdown(sock, SHUT_RDWR);
    close(sock);

    // Close all client sockets
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (int fd : client_fds) {
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        client_fds.clear();
    }

    // Join threads
    for (auto& t : pool) {
        if (t.joinable()) t.join();
    }

    {
        std::lock_guard<std::mutex> lk(cout_mutex);
        std::cout << "Server terminated.\n";
    }

    return 0;
}