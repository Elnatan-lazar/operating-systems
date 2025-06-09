#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/un.h>
#include <getopt.h>

#define BUFFER_SIZE 1024

static std::string client_path;

static void cleanup_client_socket() {
    unlink(client_path.c_str());
}

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
    sockaddr_storage serv_addr{};
    socklen_t serv_len;

    if (!sockfile.empty()) {
        sock = socket(AF_UNIX, SOCK_DGRAM, 0);

        // Create unique client socket file
        client_path = "/tmp/client_dgram_" + std::to_string(getpid());
        sockaddr_un client_addr{};
        client_addr.sun_family = AF_UNIX;
        strcpy(client_addr.sun_path, client_path.c_str());
        unlink(client_path.c_str());
        bind(sock, (sockaddr*)&client_addr, sizeof(client_addr));

        sockaddr_un server_addr{};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, sockfile.c_str());

        serv_len = sizeof(server_addr);
        memcpy(&serv_addr, &server_addr, serv_len);

        atexit(cleanup_client_socket);  // במקום ה-lambda
    } else {
        if (hostname.empty() || port_str.empty()) {
            std::cerr << "Hostname/port or socket file is required!" << std::endl;
            return 1;
        }
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        int err = getaddrinfo(hostname.c_str(), port_str.c_str(), &hints, &res);
        if (err != 0) {
            std::cerr << "getaddrinfo: " << gai_strerror(err) << std::endl;
            return 1;
        }
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        serv_len = res->ai_addrlen;
        memcpy(&serv_addr, res->ai_addr, serv_len);
        freeaddrinfo(res);
    }

    sockaddr_storage server_addr_copy = serv_addr;

    std::string input;
    while (true) {
        std::cout << "Enter UDP command (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        sendto(sock, input.c_str(), input.size(), 0, (sockaddr*)&server_addr_copy, serv_len);

        char buf[BUFFER_SIZE];
        int n = recvfrom(sock, buf, sizeof(buf)-1, 0, nullptr, nullptr);
        buf[n] = '\0';
        std::cout << "Server response: " << buf << std::endl;
    }

    close(sock);
    return 0;
}
