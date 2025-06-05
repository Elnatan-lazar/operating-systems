#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unordered_map>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <getopt.h>
#include <csignal>
#include <algorithm>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

std::unordered_map<std::string, unsigned long long> atom_inventory;
const unsigned long long MAX_ATOMS = 1000000000000000000ULL;

int tcp_port = -1, udp_port = -1, timeout_sec = 0;
std::string uds_stream_path, uds_dgram_path;

void print_inventory() {
    std::cout << "Inventory:" << std::endl;
    for (const auto &pair : atom_inventory)
        std::cout << pair.first << ": " << pair.second << std::endl;
    std::cout << std::endl;
}

void handle_alarm(int sig) {
    std::cout << "No activity for " << timeout_sec << " seconds. Shutting down server." << std::endl;
    exit(0);
}

void process_tcp_command(const std::string &cmd) {
    std::cout << "Received TCP command: " << cmd << std::endl;
    std::istringstream iss(cmd);
    std::string action, atom;
    unsigned long long number;
    if (iss >> action >> atom >> number && action == "ADD") {
        if (atom != "CARBON" && atom != "OXYGEN" && atom != "HYDROGEN") {
            std::cerr << "Invalid atom type!" << std::endl;
            return;
        }
        atom_inventory[atom] = std::min(MAX_ATOMS, atom_inventory[atom] + number);
        print_inventory();
    } else {
        std::cerr << "Invalid TCP command!" << std::endl;
    }
}

bool process_udp_command(const std::string &cmd) {
    std::cout << "Received UDP command: " << cmd << std::endl;
    std::istringstream iss(cmd);
    std::string action, molecule;
    unsigned long long number;

    if (!(iss >> action >> molecule) || action != "DELIVER") {
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

    if (atom_inventory["CARBON"]>=needed_C && atom_inventory["OXYGEN"]>=needed_O && atom_inventory["HYDROGEN"]>=needed_H) {
        atom_inventory["CARBON"]-=needed_C;
        atom_inventory["OXYGEN"]-=needed_O;
        atom_inventory["HYDROGEN"]-=needed_H;
        atom_inventory[molecule_name]+=number;
        print_inventory();
        return true;
    }
    std::cerr << "Not enough atoms" << std::endl;
    return false;
}

void process_console_command(const std::string &cmd) {
    unsigned long long w=atom_inventory["WATER"], c=atom_inventory["CARBON DIOXIDE"],
            a=atom_inventory["ALCOHOL"], g=atom_inventory["GLUCOSE"];
    if (cmd=="GEN SOFT DRINK")
        std::cout<<"SOFT DRINKs: "<<std::min({w,c,g})<<std::endl;
    else if (cmd=="GEN VODKA")
        std::cout<<"VODKA: "<<std::min({w,a,g})<<std::endl;
    else if (cmd=="GEN CHAMPAGNE")
        std::cout<<"CHAMPAGNE: "<<std::min({w,c,a})<<std::endl;
    else
        std::cerr << "Invalid console command!" << std::endl;
}

int main(int argc, char *argv[]) {
    int opt;
    struct option long_opts[] = {
            {"tcp-port", required_argument, nullptr, 'T'},
            {"udp-port", required_argument, nullptr, 'U'},
            {"stream-path", required_argument, nullptr, 's'},
            {"datagram-path", required_argument, nullptr, 'd'},
            {"carbon", required_argument, nullptr, 'c'},
            {"oxygen", required_argument, nullptr, 'o'},
            {"hydrogen", required_argument, nullptr, 'h'},
            {"timeout", required_argument, nullptr, 't'},
            {nullptr, 0, nullptr, 0}
    };

    // אתחול ראשוני
    tcp_port = -1;
    udp_port = -1;
    timeout_sec = 0;
    uds_stream_path.clear();
    uds_dgram_path.clear();

    while ((opt = getopt_long(argc, argv, "T:U:s:d:c:o:h:t:", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'T': tcp_port = std::stoi(optarg); break;
            case 'U': udp_port = std::stoi(optarg); break;
            case 's': uds_stream_path = optarg; break;
            case 'd': uds_dgram_path = optarg; break;
            case 'c': atom_inventory["CARBON"] = std::stoull(optarg); break;
            case 'o': atom_inventory["OXYGEN"] = std::stoull(optarg); break;
            case 'h': atom_inventory["HYDROGEN"] = std::stoull(optarg); break;
            case 't': timeout_sec = std::stoi(optarg); break;
            default:
                std::cerr << "Usage error: " << argv[0]
                          << " [-T tcp_port] [-U udp_port] [-s uds_stream_path] [-d uds_dgram_path]"
                          << " [-c carbon] [-o oxygen] [-h hydrogen] [-t timeout]" << std::endl;
                return 1;
        }
    }

    // בדיקת התנגשויות בין tcp ו uds stream
    if (tcp_port != -1 && !uds_stream_path.empty()) {
        std::cerr << "Error: Cannot specify both TCP port (-T) and UNIX stream socket (-s)." << std::endl;
        return 1;
    }
    // בדיקת התנגשויות בין udp ו uds datagram
    if (udp_port != -1 && !uds_dgram_path.empty()) {
        std::cerr << "Error: Cannot specify both UDP port (-U) and UNIX datagram socket (-d)." << std::endl;
        return 1;
    }
    // חייב להיות לפחות TCP או uds stream
    if (tcp_port == -1 && uds_stream_path.empty()) {
        std::cerr << "Error: Must specify TCP port (-T) or UNIX stream socket (-s)." << std::endl;
        return 1;
    }
    // חייב להיות לפחות UDP או uds datagram
    if (udp_port == -1 && uds_dgram_path.empty()) {
        std::cerr << "Error: Must specify UDP port (-U) or UNIX datagram socket (-d)." << std::endl;
        return 1;
    }

    if (timeout_sec > 0) {
        signal(SIGALRM, handle_alarm);
        alarm(timeout_sec);
    }

    int tcp_sock = -1;
    if (tcp_port != -1) {
        tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
        int optval = 1;
        setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(tcp_port);
        if (bind(tcp_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind tcp");
            exit(1);
        }
        if (listen(tcp_sock, MAX_CLIENTS) == -1) {
            perror("listen tcp");
            exit(1);
        }
        std::cout << "TCP socket listening on port " << tcp_port << std::endl;
    }

    int udp_sock = -1;
    if (udp_port != -1) {
        udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(udp_port);
        if (bind(udp_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind udp");
            exit(1);
        }
        std::cout << "UDP socket listening on port " << udp_port << std::endl;
    }

    int uds_stream_sock = -1;
    if (!uds_stream_path.empty()) {
        unlink(uds_stream_path.c_str());
        uds_stream_sock = socket(AF_UNIX, SOCK_STREAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, uds_stream_path.c_str(), sizeof(addr.sun_path) - 1);
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
        if (bind(uds_stream_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind uds_stream");
            exit(1);
        }
        if (listen(uds_stream_sock, MAX_CLIENTS) == -1) {
            perror("listen uds_stream");
            exit(1);
        }
        std::cout << "UNIX stream socket created at: " << uds_stream_path << std::endl;
    }

    int uds_dgram_sock = -1;
    if (!uds_dgram_path.empty()) {
        unlink(uds_dgram_path.c_str());
        uds_dgram_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, uds_dgram_path.c_str(), sizeof(addr.sun_path) - 1);
        addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
        if (bind(uds_dgram_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("bind uds_dgram");
            exit(1);
        }
        std::cout << "UNIX datagram socket created at: " << uds_dgram_path << std::endl;
    }

    fd_set master, read_fds;
    FD_ZERO(&master);
    if (tcp_sock != -1) FD_SET(tcp_sock, &master);
    if (udp_sock != -1) FD_SET(udp_sock, &master);
    if (uds_stream_sock != -1) FD_SET(uds_stream_sock, &master);
    if (uds_dgram_sock != -1) FD_SET(uds_dgram_sock, &master);
    FD_SET(STDIN_FILENO, &master);
    int fdmax = std::max({tcp_sock, udp_sock, uds_stream_sock, uds_dgram_sock, STDIN_FILENO});

    std::cout << "Server started!" << std::endl;
    print_inventory();

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, nullptr, nullptr, nullptr) == -1) {
            perror("select");
            exit(1);
        }
        if (timeout_sec > 0) alarm(timeout_sec);

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                // UDS Datagram
                if (i == uds_dgram_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_un cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int n = recvfrom(i, buf, sizeof(buf) - 1, 0, (sockaddr*)&cli_addr, &len);
                    buf[n] = '\0';
                    bool success = process_udp_command(buf);
                    std::string r = success ? "DELIVERED" : "FAILED";
                    sendto(i, r.c_str(), r.size(), 0, (sockaddr*)&cli_addr, len);
                }
                    // UDP
                else if (i == udp_sock) {
                    char buf[BUFFER_SIZE];
                    sockaddr_in cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int n = recvfrom(i, buf, sizeof(buf) - 1, 0, (sockaddr*)&cli_addr, &len);
                    buf[n] = '\0';
                    bool success = process_udp_command(buf);
                    std::string r = success ? "DELIVERED" : "FAILED";
                    sendto(i, r.c_str(), r.size(), 0, (sockaddr*)&cli_addr, len);
                }
                    // UDS Stream & TCP (accept)
                else if (i == uds_stream_sock || i == tcp_sock) {
                    sockaddr_storage cli_addr{};
                    socklen_t len = sizeof(cli_addr);
                    int new_fd = accept(i, (sockaddr*)&cli_addr, &len);
                    if (new_fd != -1) {
                        FD_SET(new_fd, &master);
                        if (new_fd > fdmax) fdmax = new_fd;
                        std::cout << "New client connected (fd: " << new_fd << ")" << std::endl;
                    }
                }
                    // STDIN
                else if (i == STDIN_FILENO) {
                    std::string line;
                    std::getline(std::cin, line);
                    process_console_command(line);
                }
                    // TCP/UDS Stream data
                else {
                    char buf[BUFFER_SIZE];
                    int n = recv(i, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        buf[n] = '\0';
                        process_tcp_command(buf);
                    }
                }
            }
        }
    }

    return 0;
}
