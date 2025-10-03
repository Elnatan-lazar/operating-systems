#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>

constexpr int PORT = 8080;
constexpr const char* SERVER_IP = "127.0.0.1";

bool validate_create(const std::string& line) {
    if (line.rfind("CREATE", 0) != 0) return false;
    std::istringstream iss(line.substr(6));
    int n;
    if (!(iss >> n) || n <= 0) return false; // invalid number of vertices

    std::string token;
    while (iss >> token) {
        size_t dash = token.find('-');
        if (dash == std::string::npos) return false; // missing dash
        std::string u_str = token.substr(0, dash);
        std::string v_str = token.substr(dash + 1);
        int u, v;
        try {
            u = std::stoi(u_str);
            v = std::stoi(v_str);
        } catch (...) {
            return false; // not integer
        }
        if (u < 0 || u >= n || v < 0 || v >= n) return false; // out of range
    }
    return true; // valid CREATE
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return 1; }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); return 1;
    }

    char buffer[4096];
    ssize_t n;

    auto read_server_block = [&]() {
        while ((n = read(sock_fd, buffer, sizeof(buffer)-1)) > 0) {
            buffer[n] = '\0';
            std::cout << buffer;
            if (n < sizeof(buffer)-1) break;
            if (buffer[n-1] == '\n') break;
        }
    };

    // Print welcome block once
    read_server_block();

    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) continue;

        if (line.rfind("CREATE", 0) == 0 && !validate_create(line)) {
            std::cout << "Invalid CREATE format. Usage: CREATE <num_vertices> X-Y [X2-Y2 ...]\n";
            continue; // skip sending to server
        }

        line += "\n";
        if (write(sock_fd, line.c_str(), line.size()) < 0) {
            perror("write"); break;
        }

        read_server_block();

        if (line == "QUIT\n") {
            std::cout << "[Client] Disconnected from server.\n";
            break;
        }

        if (line == "Q\n") {
            std::cout << "[Client] Server shutdown requested. Exiting.\n";
            break;
        }
    }

    close(sock_fd);
    return 0;
}
