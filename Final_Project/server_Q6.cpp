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
#include <chrono>
#include <thread>

constexpr int PORT = 8080;
constexpr int TIMEOUT_SEC = 30;

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

// Send string to client
bool send_block(int fd, const std::string &block) {
    const char *p = block.c_str();
    size_t rem = block.size();
    while (rem) {
        ssize_t sent = write(fd, p, rem);
        if (sent <= 0) return false;
        p   += sent;
        rem -= sent;
    }
    return true;
}

int main() {
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
    auto last_connection = std::chrono::steady_clock::now();

    while (true) {
        sockaddr_in cli_addr;
        socklen_t   cli_len = sizeof(cli_addr);

        fd_set set;
        FD_ZERO(&set);
        FD_SET(server_fd, &set);
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int rv = select(server_fd + 1, &set, nullptr, nullptr, &timeout);
        if (rv == -1) { perror("select"); break; }
        else if (rv == 0) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_connection).count() >= TIMEOUT_SEC) {
                std::cout << "No client connected for " << TIMEOUT_SEC << " seconds. Shutting down server.\n";
                break;
            }
            continue;
        }

        int client_fd = accept(server_fd, (sockaddr*)&cli_addr, &cli_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        last_connection = std::chrono::steady_clock::now();
        std::cout << "Client connected\n";

        // send welcome block once
        send_block(client_fd,
            "WELCOME to Euler-Server\n"
            "Available commands:\n"
            "  CREATE <num_vertices> X-Y [X2-Y2 ...]\n"
            "  CHECK\n"
            "  RESET\n"
            "  QUIT (disconnect client)\n"
            "  Q (shutdown server)\n"
        );

        std::unique_ptr<Graph> graph;
        std::string line;
        while (recv_line(client_fd, line)) {
            if (line.empty()) continue;

            if (line == "Q") {
                send_block(client_fd, "Server shutting down.\n");
                close(client_fd);
                close(server_fd);
                std::cout << "Server shutdown requested by client.\n";
                return 0;
            }

            if (line.rfind("CREATE ", 0) == 0) {
                if (graph) {
                    send_block(client_fd, "A graph already exists. Use RESET first.\n");
                    continue;
                }

                // CREATE is already validated by client, just parse and build
                std::istringstream iss(line.substr(7));
                int n;
                iss >> n;
                graph.reset(new Graph(n, false));

                std::string token;
                while (iss >> token) {
                    size_t dash = token.find('-');
                    int u = std::stoi(token.substr(0, dash));
                    int v = std::stoi(token.substr(dash + 1));
                    graph->addEdge(u, v);
                }

                send_block(client_fd, "A graph was created!\n");

            } else if (line == "CHECK") {
                if (!graph) {
                    send_block(client_fd, "No graph created yet. Use CREATE first.\n");
                    continue;
                }
                if (!hasEulerianCircuit(*graph)) {
                    send_block(client_fd, "There isn't an Euler circle\n");
                } else {
                    auto circuit = getEulerianCircuit(*graph);
                    std::ostringstream oss;
                    oss << "There is an Euler circle: ";
                    for (size_t i = 0; i < circuit.size(); ++i) {
                        if (i) oss << ' ';
                        oss << circuit[i];
                    }
                    oss << "\n";
                    send_block(client_fd, oss.str());
                }

            } else if (line == "RESET") {
                if (!graph) {
                    send_block(client_fd, "No graph to reset.\n");
                    continue;
                }
                graph.reset();
                send_block(client_fd, "Reset the graph!\n");

            } else if (line == "QUIT") {
                send_block(client_fd, "Goodbye!\n");
                close(client_fd);
                std::cout << "Client disconnected\n";
                break;

            } else {
                send_block(client_fd, "Invalid command. Available: CHECK / RESET / QUIT / Q\n");
            }
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
