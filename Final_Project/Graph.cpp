#include "Graph.h"
#include <algorithm> // for std::find, std::remove

// initialize V and directed flag, allocate V empty lists
Graph::Graph(int vertices, bool isDirected)
        : V(vertices), directed(isDirected), adjList(vertices)
{}

// add edge u->v; if undirected also add v->u, but only if not already present
void Graph::addEdge(int u, int v) {
    if (u < 0 || u >= V || v < 0 || v >= V) {
        std::cerr << "Error: vertex index out of range\n";
        return;
    }
    // prevent duplicates in u->v
    auto& fromU = adjList[u];
    if (std::find(fromU.begin(), fromU.end(), v) == fromU.end()) {
        fromU.push_back(v);
    }

    // if undirected, also add v->u (no dupes)
    if (!directed) {
        auto& fromV = adjList[v];
        if (std::find(fromV.begin(), fromV.end(), u) == fromV.end()) {
            fromV.push_back(u);
        }
    }
}

// remove edge u->v; if undirected also remove v->u
void Graph::removeEdge(int u, int v) {
    if (u < 0 || u >= V || v < 0 || v >= V) {
        std::cerr << "Error: vertex index out of range\n";
        return;
    }
    auto& fromU = adjList[u];
    fromU.erase(std::remove(fromU.begin(), fromU.end(), v), fromU.end());

    if (!directed) {
        auto& fromV = adjList[v];
        fromV.erase(std::remove(fromV.begin(), fromV.end(), u), fromV.end());
    }
}

// print each vertex and its neighbors
void Graph::print() const {
    for (int i = 0; i < V; ++i) {
        std::cout << i << ":";
        for (int nbr : adjList[i]) {
            std::cout << " " << nbr;
        }
        std::cout << "\n";
    }
}

int Graph::getNumVertices() const {
    return V;
}

const std::vector<std::vector<int>>& Graph::getAdjList() const {
    return adjList;
}
