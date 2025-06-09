#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <udp_port>" << std::endl;
        return 1;
    }
    const char *hostname = argv[1];
    int port = std::stoi(argv[2]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    hostent *server = gethostbyname(hostname);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    std::string input;
    while (true) {
        std::cout << "Enter UDP command (or 'exit'): ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        sendto(sock, input.c_str(), input.size(), 0, (sockaddr*)&serv_addr, sizeof(serv_addr));

        char buf[BUFFER_SIZE];
        socklen_t serv_len = sizeof(serv_addr);
        int n = recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&serv_addr, &serv_len);
        buf[n] = '\0';
        std::cout << "Server response: " << buf << std::endl;
    }

    close(sock);
    return 0;
}
