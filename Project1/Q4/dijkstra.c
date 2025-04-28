#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define V 100

// Finding the vertex with the minimum distance value, 
// that doesn't in the shortest path tree
int minDistance(int dist[], int sptSet[]) {
    int min = INT_MAX, min_index = -1;
    for (int v = 0; v < V; v++) {
        if (!sptSet[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Printing output
void printSolution(int dist[]) {
    printf("Vertex \t Distance from Source\n");
    for (int i = 0; i < V; i++) {
        if (dist[i] == INT_MAX) {
            printf("%d \t\t\t\t INF\n", i);
        } else {
            printf("%d \t\t\t\t %d\n", i, dist[i]);
        }
    }
}

// Main algorithm
void dijkstra(int graph[V][V], int src) {
    int dist[V];
    int sptSet[V];

    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = 0;
    }

    dist[src] = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = minDistance(dist, sptSet);

        if (u == -1) break;

        sptSet[u] = 1;

        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX
                && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    printSolution(dist);
}

// Reading a graph from input
int readGraph(int graph[V][V]) {
    int n;
    if (scanf("%d", &n) != 1) {
        printf("Invalid input: expected number of vertices.\n");
        return 0;
    }

    if (n <= 0 || n > V) {
        printf("Invalid number of vertices: must be between 1 and %d.\n", V);
        return 0;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int weight;
            if (scanf("%d", &weight) != 1) {
                printf("Invalid input: not enough numbers.\n");
                return 0;
            }
            if (weight < 0) {
                printf("Invalid input: negative edge weight not allowed.\n");
                return 0;
            }
            graph[i][j] = weight;
        }
    }

    return n;
}

int main() {
    int graph[V][V];
    
    printf("Enter number of vertices followed by the adjacency matrix (non-negative weights):\n");

    while (1) {
        int n = readGraph(graph);
        if (n == 0) {
            printf("Error reading graph. Exiting.\n");
            break;
        }

        printf("Running Dijkstra from vertex 0:\n");
        dijkstra(graph, 0);

        printf("\nEnter another graph or Ctrl+D to exit:\n");
    }

    return 0;
}
