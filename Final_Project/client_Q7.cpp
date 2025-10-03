#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>

constexpr int PORT = 8080;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int BUFFER_SIZE = 4096;


bool validate_create(const std::string& line) {
    if (line.rfind("CREATE", 0) != 0) return false;
    std::istringstream iss(line.substr(6));
    int n;
    if (!(iss >> n) || n <= 0) return false;
    std::string edge;
    while (iss >> edge) {
        auto dash = edge.find('-');
        if (dash == std::string::npos) return false;
        try {
            int u = std::stoi(edge.substr(0, dash));
            int v = std::stoi(edge.substr(dash+1));
            if (u < 0 || u >= n || v < 0 || v >= n) return false;
        } catch (...) {
            return false;
        }
    }
    return true;
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

    char buffer[BUFFER_SIZE];
    ssize_t n;

    // Function to read entire server block
    auto read_server_block = [&]() {
        std::string msg;
        while ((n = read(sock_fd, buffer, BUFFER_SIZE-1)) > 0) {
            buffer[n] = '\0';
            msg += buffer;
            if (n < BUFFER_SIZE-1) break;
        }
        if (!msg.empty())
            std::cout << msg;
    };

    // Read initial welcome message
    read_server_block();

    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) continue;

        // Validate CREATE format
        if (line.rfind("CREATE", 0) == 0 && !validate_create(line)) {
            std::cout << "Invalid CREATE format. Usage: CREATE <num_vertices> X-Y [X2-Y2 ...]\n";
            continue;
        }

        // Send line to server with newline
        std::string send_line = line + "\n";
        if (write(sock_fd, send_line.c_str(), send_line.size()) < 0) {
            perror("write"); break;
        }

        // Read server response (full block)
        read_server_block();

        // Handle client-side exit
        if (line == "QUIT") {
            std::cout << "[Client] Disconnected from server.\n";
            break;
        }
        if (line == "Q") {
            std::cout << "[Client] Server shutdown requested. Exiting.\n";
            break;
        }
    }

    close(sock_fd);
    return 0;
}
