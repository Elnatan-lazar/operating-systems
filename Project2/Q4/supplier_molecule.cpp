/*
 * supplier_molecule.cpp
 *
 * A UDP client to request molecule delivery from the drinks bar server.
 * Usage:
 *   ./supplier_molecule -h <hostname> -p <port>
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
    hints.ai_socktype = SOCK_DGRAM;   // UDP

    int err = getaddrinfo(hostname.c_str(), port_str.c_str(), &hints, &res);
    if (err != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
        return 1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    std::string input;
    while (true) {
        std::cout << "Enter UDP command (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        sendto(sock, input.c_str(), input.size(), 0, res->ai_addr, res->ai_addrlen);

        char buf[BUFFER_SIZE];
        socklen_t serv_len = res->ai_addrlen;
        int n = recvfrom(sock, buf, sizeof(buf)-1, 0, res->ai_addr, &serv_len);
        buf[n] = '\0';
        std::cout << "Server response: " << buf << std::endl;
    }

    freeaddrinfo(res);
    close(sock);
    return 0;
}
