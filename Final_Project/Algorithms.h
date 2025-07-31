// Algorithms.h

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Graph.h"
#include <vector>

// ===== existing Eulerian =====
bool hasEulerianCircuit(const Graph& g);
std::vector<int> getEulerianCircuit(const Graph& g);

// ===== new algorithms =====

// 1) Compute MST total weight in an unweighted graph (unit weights = 1)
int computeMST(const Graph& g);

// 2) Strongly Connected Components (Kosaraju)
std::vector<std::vector<int>> getSCCs(const Graph& g);

// 3) Maximum flow from s to t (Edmonds–Karp, unit capacities)
int maxFlow(const Graph& g, int s, int t);

// 4) Hamiltonian circuit via backtracking (returns empty if none)
std::vector<int> getHamiltonianCircuit(const Graph& g);

#endif // ALGORITHMS_H
