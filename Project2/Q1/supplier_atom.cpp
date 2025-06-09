#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netdb.h>

/**
 * The main function establishes a TCP connection to the server and
 * allows the user to input commands to send to the server.
 */
int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return 1;
    }
    const char *hostname = argv[1];
    int port = std::stoi(argv[2]);

    // Create a TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Resolve the hostname to an IP address
    hostent *server = gethostbyname(hostname);
    if (server == nullptr) {
        std::cerr << "ERROR: No such host." << std::endl;
        return 1;
    }

    // Set up the server address structure
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "Connected to warehouse_atom server at " << hostname << ":" << port << std::endl;

    // User input loop
    std::string input;
    while (true) {
        std::cout << "Enter command (or 'exit' to quit): ";
        std::getline(std::cin, input);
        if (input == "exit") break;
        send(sock, input.c_str(), input.size(), 0);
    }

    close(sock);
    std::cout << "Connection closed." << std::endl;
    return 0;
}
