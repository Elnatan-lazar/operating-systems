#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstring>
#include <random>

constexpr int PORT = 8080;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int BUFFER_SIZE = 8192;

bool validate_create(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd; iss >> cmd;
    if (cmd != "CREATE") return false;

    std::string mode; int V, E;
    if (!(iss >> mode >> V >> E)) return false;
    if (V <= 0 || E < 0) return false;

    std::string tok;
    for (int i = 0; i < E; ++i) {
        if (!(iss >> tok) || tok.size() < 5 || tok.front() != '(' || tok.back() != ')') return false;
        auto comma = tok.find(',');
        try {
            int u = std::stoi(tok.substr(1, comma-1));
            int v = std::stoi(tok.substr(comma+1, tok.size()-comma-2));
            if (u < 0 || v < 0) return false;
        } catch (...) { return false; }
    }
    return true;
}

bool validate_random(const std::string& line) {
    std::istringstream iss(line);
    std::string cmd; iss >> cmd;
    if (cmd != "RANDOM") return false;

    std::string mode; int V, E; unsigned int seed;
    if (!(iss >> mode >> V >> E >> seed)) return false;
    if (V <= 0 || E < 0) return false;
    return true;
}

void read_server_block(int sock_fd) {
    char buf[BUFFER_SIZE];
    std::string msg;

    while (true) {
        ssize_t n = recv(sock_fd, buf, BUFFER_SIZE, 0);
        if (n < 0) { perror("recv"); break; }
        if (n == 0) { std::cout << "[Server closed connection]\n"; break; }

        msg.append(buf, n);

        // check if we received the full message
        size_t pos = msg.find("\n===END===\n");
        if (pos != std::string::npos) {
            std::string full_msg = msg.substr(0, pos);
            std::cout << full_msg << std::endl;
            return;
        }
    }
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) { perror("socket"); return 1; }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton"); return 1;
    }

    if (connect(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); return 1;
    }

    read_server_block(sock_fd);

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        if (line.rfind("CREATE", 0) == 0 && !validate_create(line)) {
            std::cout << "Invalid CREATE format. Usage: CREATE <directed|undirected> <V> <E> (u1,v1) ... (uE,vE)\n";
            continue;
        }
        if (line.rfind("RANDOM", 0) == 0 && !validate_random(line)) {
            std::cout << "Invalid RANDOM format. Usage: RANDOM <directed|undirected> <V> <E> <SEED>\n";
            continue;
        }

        std::string send_line = line + "\n";
        if (write(sock_fd, send_line.c_str(), send_line.size()) < 0) {
            perror("write"); break;
        }

        // Read full server response
        read_server_block(sock_fd);

        // Handle exit
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
