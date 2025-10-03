#include "Graph.h"
#include "Algorithms.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

constexpr int PORT = 8080;
constexpr int BACKLOG = 5;
constexpr int BUFFER_SIZE = 1024;

// ===== Strategies =====
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::string execute(const Graph& g, std::string& err) const = 0;
};

class EulerStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        if (!hasEulerianCircuit(g)) return "No Eulerian Circuit exists.\n";
        auto circuit = getEulerianCircuit(g);
        std::ostringstream oss;
        oss << "Eulerian Circuit:\n";
        for (int v : circuit) oss << v << " ";
        oss << "\n";
        return oss.str();
    }
};

class MSTStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        int weight = computeMST(g);
        std::ostringstream oss;
        oss << "MST weight: " << weight << "\n";
        return oss.str();
    }
};

class SCCStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        auto comps = getSCCs(g);
        std::ostringstream oss;
        oss << "SCCs:\n";
        for (size_t i = 0; i < comps.size(); ++i) {
            oss << "Component " << i << ": ";
            for (int v : comps[i]) oss << v << " ";
            oss << "\n";
        }
        return oss.str();
    }
};

class FlowStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        int src = 0, sink = g.getNumVertices() - 1;
        int flow = maxFlow(g, src, sink);
        std::ostringstream oss;
        oss << "Max flow (" << src << "->" << sink << "): " << flow << "\n";
        return oss.str();
    }
};

class HamiltonStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        std::vector<int> path;
        std::vector<bool> used(g.getNumVertices(), false);
        if (hamiltonianBacktrack(g, 0, used, path)) {
            std::ostringstream oss;
            oss << "Hamiltonian circuit:\n";
            for (int v : path) oss << v << " ";
            oss << "0\n";
            return oss.str();
        } else {
            return "No Hamiltonian circuit exists.\n";
        }
    }
private:
    bool hamiltonianBacktrack(const Graph& g, int u,
                              std::vector<bool>& used,
                              std::vector<int>& path) const {
        path.push_back(u);
        if (path.size() == (size_t)g.getNumVertices()) {
            for (int v : g.getAdjList()[u]) if (v == 0) return true;
            path.pop_back();
            return false;
        }
        used[u] = true;
        for (int v : g.getAdjList()[u]) {
            if (!used[v]) {
                if (hamiltonianBacktrack(g, v, used, path)) return true;
            }
        }
        used[u] = false;
        path.pop_back();
        return false;
    }
};

// ===== Factory =====
class StrategyFactory {
public:
    StrategyFactory() {
        registry_["euler"]    = [](){ return std::make_unique<EulerStrategy>(); };
        registry_["mst"]      = [](){ return std::make_unique<MSTStrategy>(); };
        registry_["scc"]      = [](){ return std::make_unique<SCCStrategy>(); };
        registry_["flow"]     = [](){ return std::make_unique<FlowStrategy>(); };
        registry_["hamilton"] = [](){ return std::make_unique<HamiltonStrategy>(); };
    }

    std::unique_ptr<Strategy> create(const std::string& name) const {
        auto it = registry_.find(name);
        if (it == registry_.end()) return nullptr;
        return it->second();
    }

    std::vector<std::string> available() const {
        std::vector<std::string> names;
        for (auto& p : registry_) names.push_back(p.first);
        return names;
    }

private:
    std::map<std::string, std::function<std::unique_ptr<Strategy>()>> registry_;
};

// ===== Helpers =====
static inline std::string trim(const std::string& s) {
    auto l = s.find_first_not_of(" \r\n\t");
    if (l == std::string::npos) return "";
    auto r = s.find_last_not_of(" \r\n\t");
    return s.substr(l, r - l + 1);
}

bool send_block(int fd, const std::string &block) {
    const char *p = block.c_str();
    size_t rem = block.size();
    while (rem) {
        ssize_t sent = send(fd, p, rem, 0);
        if (sent <= 0) return false;
        p   += sent;
        rem -= sent;
    }
    return true;
}

// ===== Main =====
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);
    std::cout << "Server listening on port " << PORT << "...\n";

    StrategyFactory factory;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        std::cout << "Client connected!\n";

        // welcome message
        send_block(client_fd,
            "WELCOME to Euler-Server\n"
            "Available commands:\n"
            "  CREATE <num_vertices> X-Y [X2-Y2 ...]\n"
            "  ALGO <algorithm>\n"
            "    (e.g. ALGO euler | mst | scc | flow | hamilton)\n"
            "  RESET\n"
            "  QUIT (disconnect client)\n"
            "  Q (shutdown server)\n"
        );

        Graph graph(0);
        bool has_graph = false;

        char buffer[BUFFER_SIZE];
        while (true) {
            int n = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (n <= 0) break;
            buffer[n] = '\0';
            std::string line = trim(buffer);
            if (line.empty()) continue;

            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

            if (cmd == "CREATE") {
                if (has_graph) {
                    send_block(client_fd, "A graph already exists. Use RESET first.\n");
                    continue;
                }
                int V; iss >> V;
                Graph g(V);
                std::string edge;
                while (iss >> edge) {
                    auto dash = edge.find('-');
                    int u = std::stoi(edge.substr(0, dash));
                    int v = std::stoi(edge.substr(dash+1));
                    g.addEdge(u, v);
                }
                graph = std::move(g);
                has_graph = true;
                send_block(client_fd, "Graph created with " + std::to_string(V) + " vertices.\n");
            }
            else if (cmd == "ALGO") {
                if (!has_graph) {
                    send_block(client_fd,
                        "No graph created yet. Use CREATE first.\n"
                    );
                    continue;
                }
                std::string alg;
                iss >> alg;
                std::transform(alg.begin(), alg.end(), alg.begin(), ::tolower);
                auto strat = factory.create(alg);
                if (!strat) {
                    std::ostringstream oss;
                    oss << "Unknown algorithm.\nAvailable: ";
                    for (auto& a : factory.available()) oss << a << " ";
                    oss << "\n";
                    send_block(client_fd, oss.str());
                    continue;
                }
                std::string err;
                std::string result = strat->execute(graph, err);
                if (!err.empty()) send_block(client_fd, "ERROR_ALGO\n" + err);
                else send_block(client_fd, result);
            }
            else if (cmd == "RESET") {
                if (!has_graph) {
                    send_block(client_fd,
                        "No graph created yet. Use CREATE first.\n"
                    );
                    continue;
                }
                has_graph = false;
                graph = Graph(0);
                send_block(client_fd,
                    "Server reset. Use CREATE to build a new graph.\n"
                );
            }
            else if (cmd == "QUIT") {
                send_block(client_fd, "Goodbye!\n");
                close(client_fd);
                std::cout << "Client disconnected\n";
                break; 
            }
            else if (cmd == "Q") {
                send_block(client_fd, "Server shutting down.\n");
                close(client_fd);
                close(server_fd);
                std::cout << "Server shutdown requested by client.\n";
                return 0; 
            }
            else {
                send_block(client_fd, "Invalid command. Available: CREATE.. / ALGO.. / RESET / QUIT / Q\n");
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
