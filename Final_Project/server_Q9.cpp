// server_Q9.cpp
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
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

constexpr int PORT = 8080;

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

// Thread-safe queue
template<typename T>
class SafeQueue {
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
public:
    void push(const T& item) {
        {
            std::lock_guard<std::mutex> lk(m);
            q.push(item);
        }
        cv.notify_one();
    }
    T pop() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{ return !q.empty(); });
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
void acceptLoop(int sock) {
    while (true) {
        sockaddr_in peer{};
        socklen_t len = sizeof(peer);
        int client = accept(sock, (sockaddr*)&peer, &len);
        if (client < 0) {
            perror("accept");
            continue;
        }
        // מי זה הלקוח?
        char ipstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer.sin_addr, ipstr, sizeof(ipstr));
        int portn = ntohs(peer.sin_port);
        std::cout << "[+] Connection from " << ipstr << ":" << portn << "\n";

        // שליחת ברכה והנחיות
        sendAll(client,
                "Welcome to Graph‐Server Q9 (pipeline)\n"
                "Commands:\n"
                " CREATE <directed|undirected> <V> <E> (u1,v1) ... (uE,vE)\n"
                " RANDOM <V> <E> <SEED>\n"
                " QUIT\n\n"
                "Example:\n"
                " CREATE directed 5 3 (0,1) (1,2) (3,4)\n"
                " RANDOM 5 7 12345\n\n"
                "> "
        );

        // קריאה של שורת הפקודה
        char buf[2048];
        ssize_t lenr = recv(client, buf, sizeof(buf)-1, 0);
        if (lenr <= 0) {
            std::cout << "[-] Disconnected before request arrived from "
                      << ipstr << ":" << portn << "\n";
            close(client);
            continue;
        }
        buf[lenr] = '\0';
        auto line = trim(buf);
        std::cout << "[>] Received from " << ipstr << ":" << portn
                  << " -> `" << line << "`\n";

        if (line == "QUIT") {
            sendAll(client, "Goodbye!\n");
            close(client);
            std::cout << "[-] Client " << ipstr << ":" << portn
                      << " quit\n";
            continue;
        }

        // דחיפה לשלב הפריסה
        auto rt = std::make_shared<RawTask>();
        rt->client_fd = client;
        rt->request   = line;
        q0.push(rt);
    }
}

// ====================
// Stage 1: parseLoop
// ====================
void parseLoop() {
    while (true) {
        auto rt = q0.pop();
        int client = rt->client_fd;
        std::istringstream iss(rt->request);
        std::string cmd; iss >> cmd;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

        Graph g(0);
        bool bad = false;
        bool directed = false;

        if (cmd == "CREATE") {
            // קרא דגל מכוון/לא־מכוון
            std::string dirstr;
            if (!(iss >> dirstr)) {
                bad = true;
            } else {
                std::string dl = dirstr;
                std::transform(dl.begin(), dl.end(), dl.begin(), ::tolower);
                if (dl == "directed")      directed = true;
                else if (dl == "undirected") directed = false;
                else bad = true;
            }
            int V, E;
            if (!bad && !(iss >> V >> E)) bad = true;
            if (!bad) {
                if (V <= 0 || E < 0) bad = true;
                else {
                    g = Graph(V, directed);
                    // קרא E קשתות
                    std::string tok;
                    for (int i = 0; i < E; ++i) {
                        if (!(iss >> tok)
                            || tok.size() < 5
                            || tok.front()!='('
                            || tok.back()!=')') {
                            bad = true; break;
                        }
                        auto comma = tok.find(',');
                        try {
                            int u = std::stoi(tok.substr(1, comma-1));
                            int v = std::stoi(tok.substr(comma+1,
                                                         tok.size()-comma-2));
                            if (u<0||u>=V||v<0||v>=V) { bad = true; break; }
                            g.addEdge(u, v);
                        } catch(...) {
                            bad = true; break;
                        }
                    }
                }
            }
        }
        else if (cmd == "RANDOM") {
            int V, E; unsigned int seed;
            if (!(iss >> V >> E >> seed)) bad = true;
            if (!bad) {
                if (V <= 0 || E < 0) bad = true;
                else {
                    long long maxE = 1LL*V*(V-1)/2;
                    if (E > maxE) bad = true;
                    else {
                        g = Graph(V /*, default undirected */);
                        std::mt19937 gen(seed);
                        std::set<std::pair<int,int>> edges;
                        while ((int)edges.size() < E) {
                            int a = gen()%V, b = gen()%V;
                            if (a==b) continue;
                            if (a>b) std::swap(a,b);
                            edges.emplace(a,b);
                        }
                        for (auto& e: edges)
                            g.addEdge(e.first, e.second);
                    }
                }
            }
        }
        else {
            bad = true;
        }

        if (bad) {
            sendAll(client, "ERROR: invalid command or parameters\n");
            close(client);
            continue;
        }

        // הכנת Task
        auto t = std::make_shared<Task>(client, std::move(g));

        // שלב א’: רשימת סמיכות עם מציין מכוון/לא־מכוון
        t->resp << "Adjacency List ("
                << (directed ? "directed" : "undirected")
                << "):\n";
        const auto& adj = t->g.getAdjList();
        for (int i = 0; i < t->g.getNumVertices(); ++i) {
            t->resp << i << ":";
            for (int nb: adj[i]) t->resp << " " << nb;
            t->resp << "\n";
        }

        q1.push(t);
    }
}

// ====================
// Stage 2: eulerLoop
// ====================
void eulerLoop() {
    while (true) {
        auto t = q1.pop();
        std::string section;
        if (!hasEulerianCircuit(t->g))
            section = "No Eulerian Circuit exists";
        else {
            auto circ = getEulerianCircuit(t->g);
            std::ostringstream o;
            for (int v: circ) o << v << " ";
            section = o.str();
        }
        t->resp << "\n-- euler --\n" << section << "\n";
        q2.push(t);
    }
}

// ====================
// Stage 3: mstLoop
// ====================
void mstLoop() {
    while (true) {
        auto t = q2.pop();
        int w = computeMST(t->g);
        t->resp << "\n-- mst --\nMST weight = " << w << "\n";
        q3.push(t);
    }
}

// ====================
// Stage 4: sccLoop
// ====================
void sccLoop() {
    while (true) {
        auto t = q3.pop();
        auto comps = getSCCs(t->g);
        std::ostringstream o;
        o << "SCCs:";
        for (auto& c: comps) {
            o << " {";
            for (int v: c) o << v << ",";
            o << "}";
        }
        t->resp << "\n-- scc --\n" << o.str() << "\n";
        q4.push(t);
    }
}

// ====================
// Stage 5: flowLoop
// ====================
void flowLoop() {
    while (true) {
        auto t = q4.pop();
        int src = 0, dst = t->g.getNumVertices() - 1;
        int f = maxFlow(t->g, src, dst);
        t->resp << "\n-- flow --\n"
                << "MaxFlow(" << src << "->" << dst
                << ") = " << f << "\n";
        q5.push(t);
    }
}

// ====================
// Stage 6: respondLoop
// ====================
void respondLoop() {
    while (true) {
        auto t = q5.pop();
        sendAll(t->client_fd, t->resp.str());
        close(t->client_fd);
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

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(sock, SOMAXCONN) < 0) {
        perror("listen"); return 1;
    }
    std::cout << "[+] Server Q9 listening on port " << PORT << "\n";

    // השקת כל שלבי הפייפליין
    std::thread t_accept(acceptLoop, sock);
    std::thread t_parse  (parseLoop);
    std::thread t_euler  (eulerLoop);
    std::thread t_mst    (mstLoop);
    std::thread t_scc    (sccLoop);
    std::thread t_flow   (flowLoop);
    std::thread t_resp   (respondLoop);

    // הצמדת התהליכים
    t_accept.join();
    t_parse .join();
    t_euler .join();
    t_mst   .join();
    t_scc   .join();
    t_flow  .join();
    t_resp  .join();
    close(sock);
    return 0;
}
