#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Find the vertex with the minimum distance value that is not in the shortest path tree
int minDistance(int dist[], int sptSet[], int n) {
    int min = INT_MAX, min_index = -1;
    for (int v = 0; v < n; v++) {
        if (!sptSet[v] && dist[v] <= min) {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Print the output
void printSolution(int dist[], int n) {
    printf("Vertex \t Distance from Source\n");
    for (int i = 0; i < n; i++) {
        if (dist[i] == INT_MAX) {
            printf("%d \t INF\n", i);
        } else {
            printf("%d \t %d\n", i, dist[i]);
        }
    }
}

// Main algorithm
void dijkstra(int **graph, int n, int src) {
    int *dist = malloc(n * sizeof(int));
    int *sptSet = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = 0;
    }

    dist[src] = 0;

    for (int count = 0; count < n - 1; count++) {
        int u = minDistance(dist, sptSet, n);

        if (u == -1) break;

        sptSet[u] = 1;

        for (int v = 0; v < n; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX
                && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
            }
        }
    }

    printSolution(dist, n);

    free(dist);
    free(sptSet);
}

int main() {
    int n;

    printf("Enter number of vertices:\n");

    // Check if the user input is a valid integer and only one value is entered
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Invalid input: Please enter a single positive integer for the number of vertices.\n");
        return 1;
    }

    // Check if there are extra values left in the input buffer (e.g., multiple numbers or characters)
    char ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch != ' ' && ch != '\t') { // Anything other than space or tab indicates an error
            printf("Invalid input: Only one integer is allowed for the number of vertices.\n");
            return 1;
        }
    }

    // Allocate graph
    int **graph = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        graph[i] = malloc(n * sizeof(int));
    }

    printf("Enter adjacency matrix (each row of %d numbers):\n", n);
    char buffer[1024];
    for (int i = 0; i < n; i++) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Unexpected end of input at row %d.\n", i);
            // Free memory
            for (int k = 0; k < n; k++) free(graph[k]);
            free(graph);
            return 1;
        }

        int count = 0;
        char *ptr = buffer;
        while (count < n) {
            int num, charsRead;
            if (sscanf(ptr, "%d%n", &num, &charsRead) != 1) {
                break;  // Stop if we can't read another number
            }
            if (num < 0) {
                printf("Invalid input at row %d: negative weight %d.\n", i, num);
                for (int k = 0; k < n; k++) free(graph[k]);
                free(graph);
                return 1;
            }
            graph[i][count] = num;
            count++;
            ptr += charsRead;
        }

        // Check if there were too few numbers
        if (count < n) {
            printf("Not enough numbers in row %d. Expected %d, got %d.\n", i, n, count);
            for (int k = 0; k < n; k++) free(graph[k]);
            free(graph);
            return 1;
        }

        // Check if there are extra numbers
        while (*ptr != '\0') {
            if (*ptr != ' ' && *ptr != '\n' && *ptr != '\t') {
                printf("Too many values in row %d. Expected exactly %d numbers.\n", i, n);
                for (int k = 0; k < n; k++) free(graph[k]);
                free(graph);
                return 1;
            }
            ptr++;
        }
    }

    printf("Running Dijkstra from vertex 0:\n");
    dijkstra(graph, n, 0);

    // Free the graph
    for (int i = 0; i < n; i++) {
        free(graph[i]);
    }
    free(graph);

    return 0;
}
