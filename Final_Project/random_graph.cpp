// random_graph.cpp
// Generates a random undirected graph based on command-line parameters
// and computes its Eulerian circuit (if one exists).

#include "Graph.h"
#include "Algorithms.h"
#include <iostream>
#include <vector>
#include <cstdlib>      // srand, rand, strtol
#include <ctime>        // time
#include <unistd.h>     // getopt

int main(int argc, char* argv[]) {
    int V = 0;               // number of vertices
    int E = 0;               // number of edges
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    int opt;

    // Parse command-line options: -v <vertices>, -e <edges>, -s <seed>
    while ((opt = getopt(argc, argv, "v:e:s:")) != -1) {
        switch (opt) {
            case 'v': V    = std::strtol(optarg, nullptr, 10); break;
            case 'e': E    = std::strtol(optarg, nullptr, 10); break;
            case 's': seed = static_cast<unsigned int>(std::strtoul(optarg, nullptr, 10)); break;
            default:
                std::cerr << "Usage: " << argv[0]
                          << " -v <num_vertices> -e <num_edges> [-s <seed>]\n";
                return 1;
        }
    }

    // Validate inputs
    if (V <= 0 || E < 0) {
        std::cerr << "Error: Vertices must be >0 and edges must be >=0.\n";
        return 1;
    }

    // Maximum possible edges in undirected simple graph
    int maxEdges = V * (V - 1) / 2;
    // Check that requested edges do not exceed maximum
    if (E > maxEdges) {
        std::cerr << "Error: number of edges (" << E << ") exceeds maximum possible ("
                  << maxEdges << ") for " << V << " vertices.\n";
        return 1;
    }

    // Seed random number generator
    std::srand(seed);

    // Create graph
    Graph g(V);

    // Track which edges have been added to avoid duplicates
    std::vector<std::vector<bool>> added(V, std::vector<bool>(V, false));
    int count = 0;
    while (count < E) {
        int u = std::rand() % V;
        int v = std::rand() % V;
        if (u == v) continue;                    // skip self-loops
        if (added[u][v] || added[v][u]) continue; // skip duplicate edges
        g.addEdge(u, v);
        added[u][v] = added[v][u] = true;
        ++count;
    }

    // Output graph info and adjacency list
    std::cout << "Random Graph (V=" << V << ", E=" << E
              << ", seed=" << seed << ")\n";
    g.print();

    // Compute and print Eulerian circuit
    if (hasEulerianCircuit(g)) {
        auto circuit = getEulerianCircuit(g);
        std::cout << "Eulerian Circuit found:\n";
        for (int node : circuit) {
            std::cout << node << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "No Eulerian Circuit exists.\n";
    }

    return 0;
}
