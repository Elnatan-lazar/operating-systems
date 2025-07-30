#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 9034
#define BUFFER_SIZE 1024

struct Point {
    double x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator<(const Point& p) const { return x < p.x || (x == p.x && y < p.y); }
};

std::vector<Point> graph;

// Returns the help menu sent on client connect (and on unknown commands)
std::string helpText() {
    return "Valid commands:\n"
           "  Newgraph n      - create a new graph and enter n points\n"
           "  Newpoint x,y    - add a new point\n"
           "  Removepoint x,y - remove a point\n"
           "  CH              - compute convex hull and print area\n"
           "  EXIT            - disconnect from server\n";
}

double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> computeConvexHull(std::vector<Point> points) {
    std::sort(points.begin(), points.end());
    std::vector<Point> hull;
    for (const auto& p : points) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size()-2], hull[hull.size()-1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    size_t lower = hull.size();
    for (int i = static_cast<int>(points.size()) - 2; i >= 0; --i) {
        while (hull.size() > lower &&
               cross(hull[hull.size()-2], hull[hull.size()-1], points[i]) <= 0)
            hull.pop_back();
        hull.push_back(points[i]);
    }
    if (!hull.empty()) hull.pop_back();
    return hull;
}

double computePolygonArea(const std::vector<Point>& polygon) {
    double area = 0.0;
    int n = polygon.size();
    for (int i = 0; i < n; ++i) {
        const Point& p1 = polygon[i];
        const Point& p2 = polygon[(i + 1) % n];
        area += (p1.x * p2.y - p2.x * p1.y);
    }
    return std::abs(area) / 2.0;
}

bool parsePoint(const std::string& input, Point& outPoint) {
    // strip spaces, CR, LF
    std::string cleaned;
    for (char c : input) {
        if (c != ' ' && c != '\r' && c != '\n') cleaned += c;
    }
    std::istringstream iss(cleaned);
    double x, y; char comma;
    if ((iss >> x >> comma >> y) && comma == ',' && iss.eof()) {
        outPoint = {x, y};
        return true;
    }
    return false;
}

void handleClient(int client_sock) {
    char buffer[BUFFER_SIZE];
    int expectedPoints = 0;

    // Send help menu on connect
    send(client_sock, helpText().c_str(), helpText().size(), 0);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesRead = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) break;  // client disconnected

        std::istringstream iss(buffer);
        std::string cmd;
        iss >> cmd;

        std::ostringstream reply;

        // Handle EXIT command first
        if (cmd == "EXIT" || cmd == "exit") {
            reply << "Goodbye!\n";
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);
            break;  // exit the loop, close client socket
        }

        if (cmd == "Newgraph") {
            int n; iss >> n;
            graph.clear();
            expectedPoints = n;
            reply << "Insert " << n << " points in format x,y:\n";
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);

            // Read exactly n points
            while (expectedPoints > 0) {
                memset(buffer, 0, BUFFER_SIZE);
                recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
                std::string input(buffer);
                Point p;
                if (parsePoint(input, p)) {
                    if (std::find(graph.begin(), graph.end(), p) == graph.end()) {
                        graph.push_back(p);
                        reply.str("");
                        reply << "Point " << p.x << "," << p.y << " added.\n";
                        expectedPoints--;
                    } else {
                        reply.str("");
                        reply << "Point already exists.\n";
                    }
                } else {
                    reply.str("");
                    reply << "Invalid format. Please enter point as x,y\n";
                }
                send(client_sock, reply.str().c_str(), reply.str().size(), 0);
            }

        } else if (cmd == "CH") {
            auto hull = computeConvexHull(graph);
            double area = computePolygonArea(hull);
            reply << "area of the convex hull " << std::fixed << std::setprecision(6)
                  << area << "\n";
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);

        } else if (cmd == "Newpoint") {
            std::string rest; std::getline(iss, rest);
            Point p;
            if (parsePoint(rest, p)) {
                if (std::find(graph.begin(), graph.end(), p) == graph.end()) {
                    graph.push_back(p);
                    reply << "Point " << p.x << "," << p.y << " added.\n";
                } else {
                    reply << "Point already exists.\n";
                }
            } else {
                reply << "Invalid format. Use x,y\n";
            }
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);

        } else if (cmd == "Removepoint") {
            std::string rest; std::getline(iss, rest);
            Point p;
            if (parsePoint(rest, p)) {
                auto it = std::find(graph.begin(), graph.end(), p);
                if (it != graph.end()) {
                    graph.erase(it);
                    reply << "Point " << p.x << "," << p.y << " removed.\n";
                } else {
                    reply << "Point not found.\n";
                }
            } else {
                reply << "Invalid format. Use x,y\n";
            }
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);

        } else {
            // Unknown command: send error + help menu
            reply << "Unknown command.\n" << helpText();
            send(client_sock, reply.str().c_str(), reply.str().size(), 0);
        }
    }

    close(client_sock);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind"); return 1;
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen"); return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";
    while (true) {
        sockaddr_in client_addr {};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_sock >= 0) {
            std::cout << "Client connected.\n";
            handleClient(client_sock);
        }
    }
    close(server_fd);
    return 0;
}
