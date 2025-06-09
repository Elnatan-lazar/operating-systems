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

std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL; // 10^18

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
        if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
            std::cerr << "Invalid atom type! Only CARBON, OXYGEN, and HYDROGEN are allowed." << std::endl;
            return;
        }
        if (atom_inventory[atom] + number <= MAX_ATOMS) {
            atom_inventory[atom] += number;
        } else {
            atom_inventory[atom] = MAX_ATOMS;
        }
        print_inventory();
    } else {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

bool process_udp_command(const std::string &cmd) {
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (!(iss >> action)) {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    if (action != "DELIVER") {
        std::cerr << "Invalid action! need to start with DELIVER" << std::endl;
        return false;
    }

    if (!(iss >> molecule)) {
        std::cerr << "Invalid UDP command!" << std::endl;
        return false;
    }

    unsigned long long needed_C=0, needed_O=0, needed_H=0;
    std::string molecule_name = molecule;

    if (molecule == "CARBON") {
        std::string second_word;
        if (!(iss >> second_word) || second_word != "DIOXIDE") {
            std::cerr << "Invalid molecule! Expected 'CARBON DIOXIDE'" << std::endl;
            return false;
        }
        molecule_name = "CARBON DIOXIDE";
    }

    if (!(iss >> number)) {
        std::cerr << "Missing number!" << std::endl;
        return false;
    }

    if (molecule_name == "WATER") {
        needed_H=2*number; needed_O=1*number;
    } else if (molecule_name == "ALCOHOL") {
        needed_C=2*number; needed_H=6*number; needed_O=1*number;
    } else if (molecule_name == "GLUCOSE") {
        needed_C=6*number; needed_H=12*number; needed_O=6*number;
    } else if (molecule_name == "CARBON DIOXIDE") {
        needed_C=1*number; needed_O=2*number;
    } else {
        std::cerr << "Invalid molecule!" << std::endl;
        return false;
    }

    if (atom_inventory["CARBON"] >= needed_C &&
        atom_inventory["OXYGEN"] >= needed_O &&
        atom_inventory["HYDROGEN"] >= needed_H) {
        atom_inventory["CARBON"] -= needed_C;
        atom_inventory["OXYGEN"] -= needed_O;
        atom_inventory["HYDROGEN"] -= needed_H;
        atom_inventory[molecule_name] += number;
        print_inventory();
        return true;
    } else {
        std::cerr << "Not enough atoms" << std::endl;
        return false;
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

    fd_set master_set, read_fds;
    FD_ZERO(&master_set);
    FD_SET(tcp_sock, &master_set);
    FD_SET(udp_sock, &master_set);
    FD_SET(STDIN_FILENO, &master_set);  // Added stdin monitoring
    int fdmax = std::max(tcp_sock, std::max(udp_sock, STDIN_FILENO));


    std::cout << "Server started! TCP port: " << tcp_port << ", UDP port: " << udp_port << std::endl;

    bool running = true;
    while (running) {
        read_fds = master_set;
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
                        FD_SET(new_fd, &master_set);
                        if (new_fd > fdmax) fdmax = new_fd;
                        std::cout << "New TCP client connected." << std::endl;
                    }
                } else if (i == udp_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t len = sizeof(client_addr);
                    int n = recvfrom(udp_sock, buf, sizeof(buf)-1, 0, (sockaddr*)&client_addr, &len);
                    buf[n] = '\0';

                    std::string response = process_udp_command(std::string(buf)) ? "DELIVERED" : "FAILED";
                    sendto(udp_sock, response.c_str(), response.size(), 0, (sockaddr*)&client_addr, len);
                } else if (i == STDIN_FILENO) {
                    std::string input_line;
                    if (std::getline(std::cin, input_line)) {
                        if (input_line == "exit") {
                            std::cout << "Exit command received. Shutting down server." << std::endl;
                            running = false;
                            break;
                        } else {
                            std::cout << "Unknown command: " << input_line << std::endl;
                        }
                    } else {
                        std::cout << "Input closed. Shutting down server." << std::endl;
                        running = false;
                        break;
                    }
                } else {
                    char buf[BUFFER_SIZE];
                    int nbytes = recv(i, buf, sizeof(buf)-1, 0);
                    if (nbytes <= 0) {
                        std::cout << "TCP client disconnected." << std::endl;
                        close(i);
                        FD_CLR(i, &master_set);
                    } else {
                        buf[nbytes] = '\0';
                        process_tcp_command(std::string(buf));
                    }
                }
            }
        }
    }

    close(tcp_sock);
    close(udp_sock);

    std::cout << "Server has shut down gracefully." << std::endl;
    return 0;
}
