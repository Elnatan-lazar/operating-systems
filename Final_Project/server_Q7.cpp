// server_Q7.cpp
// Server for Stage 7: handles CREATE and algorithm requests via Factory+Strategy

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
constexpr int BACKLOG = 1;
constexpr int BUFFER_SIZE = 1024;

// Strategy interface
class Strategy {
public:
    virtual ~Strategy() = default;
    // err: output parameter for error messages; return empty on error
    virtual std::string execute(const Graph& g, std::string& err) const = 0;
};

// Eulerian circuit strategy
class EulerStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        if (!hasEulerianCircuit(g))
            return "No Eulerian Circuit exists.\n";
        auto circuit = getEulerianCircuit(g);
        std::ostringstream oss;
        oss << "Eulerian Circuit:\n";
        for (int v : circuit) {
            oss << v << " ";
        }
        oss << "\n";
        return oss.str();
    }
};

// Minimum spanning tree weight strategy (Kruskal/Prim)
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

// Strongly connected components strategy (Kosaraju/Tarjan)
class SCCStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        auto comps = getSCCs(g);
        std::ostringstream oss;
        oss << "SCCs:\n";
        for (size_t i = 0; i < comps.size(); ++i) {
            oss << "Component " << i << ": ";
            for (int v : comps[i]) {
                oss << v << " ";
            }
            oss << "\n";
        }
        return oss.str();
    }
};

// Maximum flow strategy (Ford-Fulkerson / Edmonds-Karp)
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

// Hamiltonian circuit existence strategy (backtracking)
class HamiltonStrategy : public Strategy {
public:
    std::string execute(const Graph& g, std::string& err) const override {
        (void)err;
        std::vector<int> path;
        std::vector<bool> used(g.getNumVertices(), false);
        if (hamiltonianBacktrack(g, 0, used, path)) {
            std::ostringstream oss;
            oss << "Hamiltonian circuit:\n";
            for (int v : path) {
                oss << v << " ";
            }
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
            // check edge back to 0
            for (int v : g.getAdjList()[u]) {
                if (v == 0) return true;
            }
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

// Factory for strategies
class StrategyFactory {
public:
    StrategyFactory() {
        registry_["euler"] = [](){ return std::make_unique<EulerStrategy>(); };
        registry_["mst"]   = [](){ return std::make_unique<MSTStrategy>(); };
        registry_["scc"]   = [](){ return std::make_unique<SCCStrategy>(); };
        registry_["flow"]  = [](){ return std::make_unique<FlowStrategy>(); };
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

// Trim helpers
static inline std::string trim(const std::string& s) {
    auto l = s.find_first_not_of(" \r\n\t");
    if (l == std::string::npos) return "";
    auto r = s.find_last_not_of(" \r\n\t");
    return s.substr(l, r - l + 1);
}

int main() {
    // set up listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);
    std::cout << "Server listening on port " << PORT << "...\n";

    int client_fd = accept(server_fd, nullptr, nullptr);
    auto send_line = [&](const std::string& msg) {
        send(client_fd, msg.c_str(), msg.size(), 0);
    };

    // welcome message
    send_line("WELCOME to Graph-Algorithms Server Q7\n");
    send_line("Available commands:\n");
    send_line("  CREATE <num_vertices> <u1> <v1> [<u2> <v2> ...]\n");
    send_line("  ALGO <algorithm>\n");
    send_line("    (e.g. ALGO euler | mst | scc | flow | hamilton)\n");
    send_line("  RESET\n");
    send_line("  QUIT\n");

    StrategyFactory factory;
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
            int V;
            iss >> V;
            if (!iss || V <= 0) {
                send_line("ERROR_BAD_CREATE\nUsage: CREATE <num_vertices> <u1> <v1> ...\n");
                continue;
            }
            Graph g(V);
            int u,v;
            bool bad = false;
            while (iss >> u >> v) {
                if (u < 0 || u >= V || v < 0 || v >= V) { bad = true; break; }
                g.addEdge(u, v);
            }
            if (bad) {
                send_line("ERROR_BAD_VERTEX\nVertices must be in [0.." + std::to_string(V-1) + "]\n");
                continue;
            }
            graph = std::move(g);
            has_graph = true;
            send_line("Graph created with " + std::to_string(V) + " vertices.\n");
        }
        else if (cmd == "ALGO") {
            if (!has_graph) {
                send_line("ERROR_NO_GRAPH\nPlease CREATE a graph first.\n");
                continue;
            }
            std::string alg;
            iss >> alg;
            std::transform(alg.begin(), alg.end(), alg.begin(), ::tolower);
            auto strat = factory.create(alg);
            if (!strat) {
                send_line("ERROR_UNKNOWN_ALGO\nAvailable: ");
                for (auto& a : factory.available()) send_line(a + " ");
                send_line("\n");
                continue;
            }
            std::string err;
            std::string result = strat->execute(graph, err);
            if (!err.empty()) {
                send_line("ERROR_ALGO\n" + err);
            } else {
                send_line(result);
            }
        }
        else if (cmd == "RESET") {
            has_graph = false;
            graph = Graph(0);
            send_line("Server reset. Use CREATE to build a new graph.\n");
        }
        else if (cmd == "QUIT") {
            send_line("Goodbye!\n");
            break;
        }
        else {
            send_line("ERROR_UNKNOWN_COMMAND\n");
            send_line("Available commands: CREATE, ALGO, RESET, QUIT\n");
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
