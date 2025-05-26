/*
 * bar_drinks.cpp
 *
 * A TCP/UDP server to manage a warehouse of atoms and check drink/molecule availability.
 *
 * TCP: ADD CARBON/OXYGEN/HYDROGEN <number> (updates inventory).
 * UDP: DELIVER ... (just checks how many can be created, no reduction).
 * Keyboard: GEN ... (checks how many drinks can be created, no reduction).
 *
 * Usage:
 *   ./bar_drinks <tcp_port> <udp_port>
 */

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
#include <algorithm>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL;

void print_inventory() {
    std::cout << "Inventory:" << std::endl;
    for (const auto &pair : atom_inventory) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

void process_tcp_command(const std::string &cmd) {
    std::istringstream iss(cmd);
    std::string action, atom;
    unsigned long long number;

    if (iss >> action >> atom >> number && action == "ADD") {
        if (atom == "CARBON" || atom == "OXYGEN" || atom == "HYDROGEN") {
            if (atom_inventory[atom] + number <= MAX_ATOMS) {
                atom_inventory[atom] += number;
            } else {
                atom_inventory[atom] = MAX_ATOMS;
            }
            print_inventory();
        } else {
            std::cerr << "Invalid atom type!" << std::endl;
        }
    } else {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

unsigned long long calculate_molecule(const std::string &molecule) {
    if (molecule == "WATER") {
        return std::min(atom_inventory["OXYGEN"] / 1, atom_inventory["HYDROGEN"] / 2);
    }
    if (molecule == "CARBON DIOXIDE") {
        return std::min(atom_inventory["CARBON"] / 1, atom_inventory["OXYGEN"] / 2);
    }
    if (molecule == "ALCOHOL") {
        return std::min({atom_inventory["CARBON"] / 2, atom_inventory["HYDROGEN"] / 6, atom_inventory["OXYGEN"] / 1});
    }
    if (molecule == "GLUCOSE") {
        return std::min({atom_inventory["CARBON"] / 6, atom_inventory["HYDROGEN"] / 12, atom_inventory["OXYGEN"] / 6});
    }
    return 0;
}

bool process_udp_command(const std::string &cmd, std::string &response) {
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (iss >> action >> molecule >> number && action == "DELIVER") {
        std::string second;
        if (molecule == "CARBON" && iss >> second && second == "DIOXIDE") {
            molecule = "CARBON DIOXIDE";
        }

        if (molecule == "WATER" || molecule == "CARBON DIOXIDE" || molecule == "ALCOHOL" || molecule == "GLUCOSE") {
            unsigned long long available = calculate_molecule(molecule);
            if (available >= number) {
                response = "CAN DELIVER " + std::to_string(number) + " " + molecule + " (max available: " + std::to_string(available) + ")";
                return true;
            } else {
                response = "CANNOT DELIVER. Max available: " + std::to_string(available);
                return false;
            }
        } else {
            response = "Invalid molecule!";
            return false;
        }
    }

    response = "Invalid UDP command!";
    return false;
}

void process_console_command(const std::string &cmd) {
    unsigned long long water = calculate_molecule("WATER");
    unsigned long long carbon_dioxide = calculate_molecule("CARBON DIOXIDE");
    unsigned long long alcohol = calculate_molecule("ALCOHOL");
    unsigned long long glucose = calculate_molecule("GLUCOSE");

    if (cmd == "GEN SOFT DRINK") {
        unsigned long long count = std::min({water, carbon_dioxide, glucose});
        std::cout << "SOFT DRINKs available: " << count << std::endl;
    } else if (cmd == "GEN VODKA") {
        unsigned long long count = std::min({water, alcohol, glucose});
        std::cout << "VODKA drinks available: " << count << std::endl;
    } else if (cmd == "GEN CHAMPAGNE") {
        unsigned long long count = std::min({water, carbon_dioxide, alcohol});
        std::cout << "CHAMPAGNE drinks available: " << count << std::endl;
    } else {
        std::cerr << "Invalid console command!" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <tcp_port> <udp_port>" << std::endl;
        return 1;
    }
    int tcp_port = std::stoi(argv[1]);
    int udp_port = std::stoi(argv[2]);

    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(tcp_port);
    bind(tcp_sock, (sockaddr*)&tcp_addr, sizeof(tcp_addr));
    listen(tcp_sock, MAX_CLIENTS);

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(udp_port);
    bind(udp_sock, (sockaddr*)&udp_addr, sizeof(udp_addr));

    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_SET(tcp_sock, &master);
    FD_SET(udp_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    int fdmax = std::max({tcp_sock, udp_sock, STDIN_FILENO});

    std::cout << "Bar server (check only) started on TCP port " << tcp_port << " and UDP port " << udp_port << std::endl;

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == tcp_sock) {
                    sockaddr_in cli_addr{};
                    socklen_t cli_len = sizeof(cli_addr);
                    int new_fd = accept(tcp_sock, (sockaddr*)&cli_addr, &cli_len);
                    if (new_fd != -1) {
                        FD_SET(new_fd, &master);
                        if (new_fd > fdmax) fdmax = new_fd;
                        std::cout << "New TCP client connected." << std::endl;
                    }
                } else if (i == udp_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_sock, buf, sizeof(buf)-1, 0, (sockaddr*)&client_addr, &len);
                    buf[n] = '\0';
                    std::string response;
                    process_udp_command(buf, response);
                    sendto(udp_sock, response.c_str(), response.size(), 0, (sockaddr*)&client_addr, len);
                } else if (i == STDIN_FILENO) {
                    std::string line;
                    std::getline(std::cin, line);
                    process_console_command(line);
                } else {
                    char buf[BUFFER_SIZE];
                    int nbytes = recv(i, buf, sizeof(buf)-1, 0);
                    if (nbytes <= 0) {
                        std::cout << "TCP client disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        buf[nbytes] = '\0';
                        process_tcp_command(buf);
                    }
                }
            }
        }
    }
    return 0;
}
