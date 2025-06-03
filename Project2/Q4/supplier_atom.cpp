/*
 * supplier_atom.cpp
 *
 * A TCP client to add atoms or molecules to the drinks bar server.
 * Usage:
 *   ./supplier_atom -h <hostname> -p <port>
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <getopt.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    std::string hostname;
    std::string port_str;

    int opt;
    struct option long_opts[] = {
            {"host", required_argument, nullptr, 'h'},
            {"port", required_argument, nullptr, 'p'},
            {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc, argv, "h:p:", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'h': hostname = optarg; break;
            case 'p': port_str = optarg; break;
            default:
                std::cerr << "Usage: " << argv[0] << " -h <hostname> -p <port>" << std::endl;
                return 1;
        }
    }

    if (hostname.empty() || port_str.empty()) {
        std::cerr << "Hostname and port are required!" << std::endl;
        return 1;
    }

    // getaddrinfo
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_UNSPEC;      // IPv4 או IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    int err = getaddrinfo(hostname.c_str(), port_str.c_str(), &hints, &res);
    if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        return 1;
    }

    // יצירת socket וחיבור
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        freeaddrinfo(res);
        close(sock);
        return 1;
    }

    freeaddrinfo(res);

    std::string input;
    while (true) {
        std::cout << "Enter TCP command (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    return 0;
}
