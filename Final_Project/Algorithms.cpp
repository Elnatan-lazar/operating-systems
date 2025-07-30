#include "Algorithms.h"
#include <algorithm>
#include <stack>
#include <iterator>  // for std::reverse

// ===== Eulerian Circuit for undirected graphs =====

// Check if an undirected graph has an Eulerian circuit.
// 1) find a start vertex with non-zero degree
// 2) check connectivity among non-zero vertices
// 3) ensure every vertex has even degree
bool hasEulerianCircuit(const Graph& g) {
    int V = g.getNumVertices();
    const auto& adj = g.getAdjList();

    // 1) find start
    int start = -1;
    for (int i = 0; i < V; ++i) {
        if (!adj[i].empty()) {
            start = i;
            break;
        }
    }
    if (start == -1)
        return true;  // no edges at all

    // 2) connectivity via DFS
    std::vector<bool> visited(V, false);
    std::stack<int> st;
    st.push(start);
    visited[start] = true;

    while (!st.empty()) {
        int u = st.top(); st.pop();
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                st.push(v);
            }
        }
    }
    // ensure all non-zero-degree vertices were reached
    for (int i = 0; i < V; ++i) {
        if (!adj[i].empty() && !visited[i])
            return false;
    }

    // 3) all degrees must be even
    for (int i = 0; i < V; ++i) {
        if (adj[i].size() % 2 != 0)
            return false;
    }
    return true;
}

// Compute the Eulerian circuit of an undirected graph.
// Uses Hierholzer’s algorithm:
// 1) copy adjacency, start from a vertex with edges
// 2) follow unused edges, pushing vertices to stack
// 3) backtrack when stuck, appending to circuit
std::vector<int> getEulerianCircuit(const Graph& g) {
    std::vector<int> circuit;
    if (!hasEulerianCircuit(g))
        return circuit;  // empty if none

    int V = g.getNumVertices();
    auto localAdj = g.getAdjList();  // make a modifiable copy
    std::stack<int> st;

    // find a start vertex
    int start = 0;
    for (int i = 0; i < V; ++i) {
        if (!localAdj[i].empty()) {
            start = i;
            break;
        }
    }
    st.push(start);

    // Hierholzer’s loop
    while (!st.empty()) {
        int u = st.top();
        if (!localAdj[u].empty()) {
            int v = localAdj[u].back();
            st.push(v);
            // remove edge u–v both ways
            localAdj[u].pop_back();
            auto& nbrs = localAdj[v];
            nbrs.erase(std::find(nbrs.begin(), nbrs.end(), u));
        } else {
            circuit.push_back(u);
            st.pop();
        }
    }
    std::reverse(circuit.begin(), circuit.end());
    return circuit;
}
