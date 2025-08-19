#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

// Using an unordered_map to store any type of atom dynamically
std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL; // 10^18

/**
 * Prints the current inventory of the warehouse.
 */
void print_inventory() {
    std::cout << "Inventory:" << std::endl;
    for (const auto &pair : atom_inventory) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

/**
 * Processes a single command from a client.
 * Supported command format: ADD <ATOM_TYPE> <NUMBER>
 * Adds the specified number of atoms to the warehouse, creating new types if necessary.
 *
 * @param cmd A string representing the command received from the client.
 */
void process_command(const std::string &cmd) {
    std::istringstream iss(cmd);
    std::string action, atom;
    unsigned long long number;

    // Parse the command
    if (iss >> action >> atom >> number && action == "ADD" ) {
        if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
            std::cerr << "Invalid atom type! Only CARBON, OXYGEN, and HYDROGEN are allowed." << std::endl;
            return;
        }
        // Ensure the count does not exceed the maximum allowed for each atom type
        if (atom_inventory[atom] + number <= MAX_ATOMS) {
            atom_inventory[atom] += number;
        } else {
            atom_inventory[atom] = MAX_ATOMS;
        }
        print_inventory();
    } else {
        std::cerr << "Invalid command! need to be: ADD <ATOM TYPE> <NUMBER>" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::stoi(argv[1]);

    // Create a TCP socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    // Set up the server address structure
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind the socket to the specified port
    if (bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Start listening for incoming connections
    if (listen(listen_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        return 1;
    }

    // Initialize fd_set for select()
    fd_set master_set, read_fds;
    FD_ZERO(&master_set);
    FD_SET(listen_fd, &master_set);
    FD_SET(STDIN_FILENO, &master_set);
    int fdmax = std::max(listen_fd, STDIN_FILENO);

    std::cout << "Warehouse server (dynamic atoms) started on port " << port << "..." << std::endl;

    // Main server loop
    bool running = true;
    while (running) {
        read_fds = master_set;

        // Use select() to wait for activity on any socket or stdin
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            exit(1);
        }

        // Check each fd for activity
        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listen_fd) {
                    // New client connection
                    sockaddr_in cli_addr{};
                    socklen_t cli_len = sizeof(cli_addr);
                    int new_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);
                    if (new_fd != -1) {
                        FD_SET(new_fd, &master_set);
                        if (new_fd > fdmax) fdmax = new_fd;
                        std::cout << "New client connected." << std::endl;
                    }
                } else if (i == STDIN_FILENO) {
                    // Input from keyboard
                    std::string input_line;
                    if (std::getline(std::cin, input_line)) {
                        if (input_line == "exit") {
                            std::cout << "Exit command received. Shutting down server." << std::endl;
                            running = false;
                            break;  // break out of the for loop
                        } else {
                            std::cout << "Unknown command: " << input_line << std::endl;
                        }
                    } else {
                        std::cout << "Input closed. Shutting down server." << std::endl;
                        running = false;
                        break;
                    }
                } else {
                    // Data from existing client
                    char buf[BUFFER_SIZE];
                    int nbytes = recv(i, buf, sizeof(buf) - 1, 0);
                    if (nbytes <= 0) {
                        // Connection closed by client
                        std::cout << "Client disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        buf[nbytes] = '\0';
                        process_command(std::string(buf));
                    }
                }
            }
        }
    }

    // Cleanup sockets on shutdown
    close(listen_fd);

    std::cout << "Server has shut down gracefully." << std::endl;
    return 0;
}
