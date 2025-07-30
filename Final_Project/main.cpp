#include "Graph.h"
#include "Algorithms.h"
#include <iostream>

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

    if (hasEulerianCircuit(g)) {
        auto circuit = getEulerianCircuit(g);
        std::cout << "Eulerian Circuit:\n";
        for (int v : circuit) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "No Eulerian Circuit exists.\n";
    }
    return 0;
}
