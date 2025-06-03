/*
 * supplier_atom.cpp
 *
 * A client to add atoms to the drinks bar server.
 * Usage:
 *   ./supplier_atom -h <hostname> -p <port>
 *   ./supplier_atom -f <socket_file>
 */

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>
#include <getopt.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    std::string hostname, port_str, sockfile;
    int opt;
    struct option long_opts[] = {
            {"host", required_argument, nullptr, 'h'},
            {"port", required_argument, nullptr, 'p'},
            {"file", required_argument, nullptr, 'f'},
            {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc, argv, "h:p:f:", long_opts, nullptr)) != -1) {
        switch (opt) {
            case 'h': hostname = optarg; break;
            case 'p': port_str = optarg; break;
            case 'f': sockfile = optarg; break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-h <hostname> -p <port>] | [-f <socket_file>]" << std::endl;
                return 1;
        }
    }

    if ((!hostname.empty() || !port_str.empty()) && !sockfile.empty()) {
        std::cerr << "Error: cannot specify both hostname/port and socket file (-f)." << std::endl;
        return 1;
    }

    int sock = -1;
    if (!sockfile.empty()) {
        std::cout << "Connecting to UNIX socket: " << sockfile << std::endl;
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, sockfile.c_str());
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
            perror("connect");
            return 1;
        }
    } else {
        if (hostname.empty() || port_str.empty()) {
            std::cerr << "Hostname/port or socket file is required!" << std::endl;
            return 1;
        }
        std::cout << "Connecting to " << hostname << ":" << port_str << std::endl;
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        int err = getaddrinfo(hostname.c_str(), port_str.c_str(), &hints, &res);
        if (err != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
            return 1;
        }
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
            perror("connect");
            freeaddrinfo(res);
            return 1;
        }
        freeaddrinfo(res);
    }

    std::string input;
    while (true) {
        std::cout << "Enter TCP command (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;
        std::cout << "Sending: [" << input << "]" << std::endl;
        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    return 0;
}
