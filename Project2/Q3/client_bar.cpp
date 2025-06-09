#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <tcp|udp> <hostname> <port>" << std::endl;
        return 1;
    }
    std::string mode = argv[1];
    const char *hostname = argv[2];
    int port = std::stoi(argv[3]);

    if (mode == "tcp") {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        hostent *server = gethostbyname(hostname);
        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);
        connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

        std::string line;
        while (true) {
            std::cout << "TCP> ";
            std::getline(std::cin, line);
            if (line == "exit") break;
            send(sock, line.c_str(), line.size(), 0);
        }
        close(sock);

    } else if (mode == "udp") {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        hostent *server = gethostbyname(hostname);
        sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        std::string line;
        while (true) {
            std::cout << "UDP> ";
            std::getline(std::cin, line);
            if (line == "exit") break;
            sendto(sock, line.c_str(), line.size(), 0, (sockaddr*)&serv_addr, sizeof(serv_addr));
            char buf[BUFFER_SIZE];
            socklen_t serv_len = sizeof(serv_addr);
            int n = recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&serv_addr, &serv_len);
            buf[n] = '\0';
            std::cout << "Server: " << buf << std::endl;
        }
        close(sock);
    } else {
        std::cerr << "Invalid mode!" << std::endl;
    }
    return 0;
}
