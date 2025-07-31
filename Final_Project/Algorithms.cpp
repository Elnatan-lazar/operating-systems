#include "Algorithms.h"
#include <algorithm>
#include <stack>
#include <iterator>  // for std::reverse
#include <queue>
#include <cstring>    // for fill

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





// 1) Compute MST total weight in an unweighted graph (unit weights = 1)
int computeMST(const Graph& g) {
    int V = g.getNumVertices();
    const auto& adj = g.getAdjList();
    std::vector<bool> visited(V, false);
    int totalWeight = 0;
    for (int i = 0; i < V; ++i) {
        if (!visited[i] && !adj[i].empty()) {
            // BFS spanning tree from component root
            std::queue<int> q;
            visited[i] = true;
            q.push(i);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (int v : adj[u]) {
                    if (!visited[v]) {
                        visited[v] = true;
                        totalWeight += 1;  // each edge has weight 1
                        q.push(v);
                    }
                }
            }
        }
    }
    return totalWeight;
}

// 2) Strongly Connected Components (Kosaraju)
static void dfsOrder(int u, const std::vector<std::vector<int>>& adj,
                     std::vector<bool>& vis, std::vector<int>& order) {
    vis[u] = true;
    for (int v : adj[u]) if (!vis[v]) dfsOrder(v, adj, vis, order);
    order.push_back(u);
}
static void dfsCollect(int u, const std::vector<std::vector<int>>& adjT,
                       std::vector<bool>& vis, std::vector<int>& comp) {
    vis[u] = true; comp.push_back(u);
    for (int v : adjT[u]) if (!vis[v]) dfsCollect(v, adjT, vis, comp);
}
std::vector<std::vector<int>> getSCCs(const Graph& g) {
    int V = g.getNumVertices();
    const auto& adj = g.getAdjList();
    std::vector<bool> visited(V, false);
    std::vector<int> order;
    for (int i = 0; i < V; ++i)
        if (!visited[i]) dfsOrder(i, adj, visited, order);
    // build transpose
    std::vector<std::vector<int>> adjT(V);
    for (int u = 0; u < V; ++u)
        for (int v : adj[u])
            adjT[v].push_back(u);
    std::fill(visited.begin(), visited.end(), false);
    std::vector<std::vector<int>> sccs;
    for (int i = V - 1; i >= 0; --i) {
        int u = order[i];
        if (!visited[u]) {
            std::vector<int> comp;
            dfsCollect(u, adjT, visited, comp);
            sccs.push_back(comp);
        }
    }
    return sccs;
}

// 3) Maximum flow from s to t (Edmonds–Karp, unit capacities)
int maxFlow(const Graph& g, int s, int t) {
    int V = g.getNumVertices();
    const auto& adj = g.getAdjList();
    std::vector<std::vector<int>> cap(V, std::vector<int>(V, 0));
    for (int u = 0; u < V; ++u)
        for (int v : adj[u])
            cap[u][v] = 1;
    int flow = 0;
    while (true) {
        std::vector<int> parent(V, -1);
        std::queue<int> q;
        q.push(s);
        parent[s] = s;
        while (!q.empty() && parent[t] == -1) {
            int u = q.front(); q.pop();
            for (int v = 0; v < V; ++v) {
                if (cap[u][v] > 0 && parent[v] == -1) {
                    parent[v] = u;
                    q.push(v);
                }
            }
        }
        if (parent[t] == -1) break;
        // augment 1 unit
        int v = t;
        while (v != s) {
            int u = parent[v];
            cap[u][v] -= 1;
            cap[v][u] += 1;
            v = u;
        }
        flow += 1;
    }
    return flow;
}

// 4) Hamiltonian circuit via backtracking
static bool hamiltonianDFS(int u, const Graph& g,
                           std::vector<bool>& vis,
                           std::vector<int>& path,
                           int start) {
    vis[u] = true;
    path.push_back(u);
    if (path.size() == (size_t)g.getNumVertices()) {
        // בדיקה שקיימת חזרה ל־start
        for (int v : g.getAdjList()[u])
            if (v == start) {
                path.push_back(start);
                return true;
            }
        vis[u] = false;
        path.pop_back();
        return false;
    }
    for (int v : g.getAdjList()[u]) {
        if (!vis[v] && hamiltonianDFS(v, g, vis, path, start))
            return true;
    }
    vis[u] = false;
    path.pop_back();
    return false;
}
std::vector<int> getHamiltonianCircuit(const Graph& g) {
    int V = g.getNumVertices();
    std::vector<bool> visited(V, false);
    std::vector<int> path;
    for (int start = 0; start < V; ++start) {
        if (hamiltonianDFS(start, g, visited, path, start))
            return path;
    }
    return {};  // קיים אוילר
}

