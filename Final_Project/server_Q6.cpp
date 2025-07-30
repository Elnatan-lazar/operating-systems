// server.cpp
#include "Graph.h"
#include "Algorithms.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cstring>

constexpr int PORT = 8080;

// Read one line (terminated by '\n')
bool recv_line(int fd, std::string &out) {
    out.clear();
    char c;
    while (true) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) return false;  // error or client closed
        if (c == '\n') break;
        out += c;
    }
    return true;
}

// Send a line + '\n'
bool send_line(int fd, const std::string &line) {
    std::string tmp = line + "\n";
    const char *p = tmp.c_str();
    size_t rem = tmp.size();
    while (rem) {
        ssize_t sent = write(fd, p, rem);
        if (sent <= 0) return false;
        p   += sent;
        rem -= sent;
    }
    return true;
}

int main() {
    // 1. create listening socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen"); close(server_fd); return 1;
    }
    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        sockaddr_in cli_addr;
        socklen_t   cli_len = sizeof(cli_addr);
        int client_fd = accept(server_fd, (sockaddr*)&cli_addr, &cli_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        std::cout << "Client connected\n";

        // 0. send welcome & usage
        send_line(client_fd, "WELCOME to Euler-Server");
        send_line(client_fd, "Available commands:");
        send_line(client_fd, "  CREATE <num_vertices> <u1> <v1> [<u2> <v2> ...]");
        send_line(client_fd, "  CHECK");
        send_line(client_fd, "  RESET");
        send_line(client_fd, "  QUIT");

        std::unique_ptr<Graph> graph;  // will hold our graph once created
        std::string line;
        while (recv_line(client_fd, line)) {
            if (line.rfind("CREATE ", 0) == 0) {
                // parse CREATE
                std::istringstream iss(line.substr(7));
                int n;
                if (!(iss >> n) || n < 0) {
                    send_line(client_fd, "ERROR_BAD_FORMAT");
                    send_line(client_fd, "Usage: CREATE <num_vertices> <u1> <v1> [<u2> <v2> ...]");
                    continue;
                }
                graph.reset(new Graph(n, false));

                int u, v;
                bool bad = false;
                while (iss >> u >> v) {
                    if (u<0||u>=n||v<0||v>=n) { bad = true; break; }
                    graph->addEdge(u, v);
                }
                if (bad) {
                    graph.reset();
                    send_line(client_fd, "ERROR_BAD_VERTEX");
                    send_line(client_fd, "Vertices must be in [0.." + std::to_string(n-1) + "]");
                } else {
                    send_line(client_fd, "GRAPH_CREATED");
                }

            } else if (line == "CHECK") {
                if (!graph) {
                    send_line(client_fd, "ERROR_NO_GRAPH");
                    send_line(client_fd, "First run CREATE command");
                    continue;
                }
                // check Eulerian circuit
                if (!hasEulerianCircuit(*graph)) {
                    send_line(client_fd, "NO_EULERIAN_CIRCUIT");
                } else {
                    auto circuit = getEulerianCircuit(*graph);
                    std::ostringstream oss;
                    for (size_t i = 0; i < circuit.size(); ++i) {
                        if (i) oss << ' ';
                        oss << circuit[i];
                    }
                    send_line(client_fd, oss.str());
                }

            } else if (line == "RESET") {
                graph.reset();
                send_line(client_fd, "GRAPH_RESET");

            } else if (line == "QUIT") {
                send_line(client_fd, "BYE");
                break;

            } else {
                // unknown command
                send_line(client_fd, "ERROR_UNKNOWN_COMMAND");
                send_line(client_fd, "Available: CREATE, CHECK, RESET, QUIT");
            }
        }

        close(client_fd);
        std::cout << "Client disconnected\n";
    }

    close(server_fd);
    return 0;
}
