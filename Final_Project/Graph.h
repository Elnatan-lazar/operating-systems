#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <iostream>

// Simple adjacency-list graph implementation
class Graph {
public:
    // Constructor: number of vertices, directed or not
    explicit Graph(int vertices, bool directed = false);

    // add edge u->v (and v->u if undirected)
    void addEdge(int u, int v);

    // remove edge u->v (and v->u if undirected)
    void removeEdge(int u, int v);

    // print the adjacency list to stdout
    void print() const;

    // getters
    int getNumVertices() const;
    const std::vector<std::vector<int>>& getAdjList() const;

private:
    int V;                                  // number of vertices
    bool directed;                         // directed vs. undirected
    std::vector<std::vector<int>> adjList; // adjacency lists
};

#endif // GRAPH_H
