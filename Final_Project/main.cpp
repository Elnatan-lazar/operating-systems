// main.cpp (מעודכן – מריץ את כל האלגוריתמים)

#include "Graph.h"
#include "Algorithms.h"
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 3 || ((argc - 2) % 2 != 0)) {
        std::cout << "Usage: " << argv[0]
                  << " <num_vertices> <u1> <v1> [<u2> <v2> ...]\n";
        return 1;
    }

    int V = std::stoi(argv[1]);
    Graph g(V);

    for (int i = 2; i + 1 < argc; i += 2) {
        int u = std::stoi(argv[i]);
        int v = std::stoi(argv[i + 1]);
        g.addEdge(u, v);
    }

    std::cout << "Adjacency list:\n";
    g.print();

    // 1) Eulerian
    if (hasEulerianCircuit(g)) {
        auto circ = getEulerianCircuit(g);
        std::cout << "Eulerian Circuit:\n";
        for (int v : circ) std::cout << v << " ";
        std::cout << "\n";
    } else {
        std::cout << "No Eulerian Circuit exists.\n";
    }

    // 2) MST
    std::cout << computeMST(g) << " = MST total weight\n";

    // 3) SCC
    auto sccs = getSCCs(g);
    std::cout << "SCCs:\n";
    for (auto& comp : sccs) {
        std::cout << "  [";
        for (size_t i = 0; i < comp.size(); ++i) {
            std::cout << comp[i] << (i+1 < comp.size() ? ", " : "");
        }
        std::cout << "]\n";
    }

    // 4) Max flow 0->V-1
    int flow = maxFlow(g, 0, V-1);
    std::cout << "Max Flow (0->" << V-1 << "): " << flow << "\n";

    // 5) Hamiltonian
    auto ham = getHamiltonianCircuit(g);
    if (ham.empty()) {
        std::cout << "No Hamiltonian Circuit exists.\n";
    } else {
        std::cout << "Hamiltonian Circuit:\n";
        for (int v : ham) std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}
