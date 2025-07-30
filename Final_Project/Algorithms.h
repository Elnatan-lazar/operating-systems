#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Graph.h"
#include <vector>

// Check if an undirected graph has an Eulerian circuit.
// Returns true if every non-zero-degree vertex is connected
// and all vertices have even degree.
bool hasEulerianCircuit(const Graph& g);

// Compute the Eulerian circuit of an undirected graph.
// If no circuit exists, returns an empty vector.
// Otherwise returns the sequence of vertices in order.
std::vector<int> getEulerianCircuit(const Graph& g);

// → כאן תוכל להוסיף בעתיד פונקציות נוספות, למשל:
// bool hasHamiltonianCircuit(const Graph& g);
// std::vector<int> getHamiltonianCircuit(const Graph& g);
// int computeMST(const Graph& g);
// וכו׳…

#endif // ALGORITHMS_H
