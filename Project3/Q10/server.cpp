#include "../Q8/proactor.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <atomic>

struct Point { double x, y; };
std::vector<Point> graph;
std::unordered_map<int, int> statePts;
std::unordered_map<int, int> clientIds;
std::mutex graphMutex;
std::atomic<bool> serverRunning{true};
int graphOwnerFd = -1;
int nextClientId = 1;
// Producer–Consumer variables
pthread_mutex_t ch_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ch_cond = PTHREAD_COND_INITIALIZER;
bool wasAbove100 = false; 

bool parsePoint(const std::string& s, Point& out) {
    std::string cleaned;
    for (char c : s) if (c != ' ' && c != '\n' && c != '\r') cleaned += c;
    std::istringstream iss(cleaned);
    char comma;
    return (iss >> out.x >> comma >> out.y) && comma == ',' && iss.eof();
}

void sendStr(int fd, const std::string& msg) {
    send(fd, msg.c_str(), msg.length(), 0);
}

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

std::vector<Point> computeConvexHull(std::vector<Point> pts) {
    std::sort(pts.begin(), pts.end(), [](const Point& a, const Point& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    std::vector<Point> h;
    for (auto& p : pts) {
        while (h.size() >= 2 && cross(h[h.size()-2], h[h.size()-1], p) <= 0)
            h.pop_back();
        h.push_back(p);
    }
    size_t lo = h.size();
    for (int i = pts.size() - 2; i >= 0; --i) {
        while (h.size() > lo && cross(h[h.size()-2], h[h.size()-1], pts[i]) <= 0)
            h.pop_back();
        h.push_back(pts[i]);
    }
    if (!h.empty()) h.pop_back();
    return h;
}

double computeArea(const std::vector<Point>& poly) {
    double area = 0;
    for (size_t i = 0; i < poly.size(); ++i) {
        const Point& p1 = poly[i];
        const Point& p2 = poly[(i+1) % poly.size()];
        area += p1.x * p2.y - p2.x * p1.y;
    }
    return std::abs(area) / 2.0;
}

void* handleClient(int fd) {
    if (clientIds.find(fd) == clientIds.end()) {
        clientIds[fd] = nextClientId++;
    }
    int clientId = clientIds[fd];

    sendStr(fd, helpText());
    statePts[fd] = 0;
    char buf[512];

    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n <= 0) {
            std::cout << "Client " << clientId << " disconnected.\n";
            std::lock_guard<std::mutex> lock(graphMutex);
            if (graphOwnerFd == fd) graphOwnerFd = -1;
            close(fd);
            return nullptr;
        }
        buf[n] = 0;
        std::istringstream iss(buf);
        std::string line;
        while (std::getline(iss, line)) {
            std::cout << "Client " << clientId << " sent: " << line << "\n";

            std::lock_guard<std::mutex> lock(graphMutex);

            if (graphOwnerFd != -1 && graphOwnerFd != fd) {
                sendStr(fd, "Another client is currently creating the graph. Please wait.\n");
                continue;
            }

            if (statePts[fd] > 0) {
                Point p;
                if (parsePoint(line, p)) {
                    graph.push_back(p);
                    sendStr(fd, "Point added.\n");
                    if (--statePts[fd] == 0) {
                        sendStr(fd, "Graph input complete.\n");
                        graphOwnerFd = -1;
                    }
                } else {
                    sendStr(fd, "Bad format, use x,y\n");
                }
                continue;
            }

            std::istringstream cmdin(line);
            std::string cmd;
            cmdin >> cmd;

            if (cmd == "EXIT") {
                sendStr(fd, "Goodbye.\n");
                std::cout << "Client " << clientId << " disconnected.\n";
                if (graphOwnerFd == fd) graphOwnerFd = -1;
                close(fd);
                return nullptr;
            } else if (cmd == "Newgraph") {
                int n;
                if (cmdin >> n) {
                    graph.clear();
                    statePts[fd] = n;
                    graphOwnerFd = fd;
                    sendStr(fd, "Insert " + std::to_string(n) + " points in x,y:\n");      
                } else {
                    sendStr(fd, "Usage: Newgraph n\n");
                }
            } else if (cmd == "Newpoint") {
                std::string rest;
                std::getline(cmdin, rest);
                Point p;
                if (parsePoint(rest, p)) {
                    graph.push_back(p);
                    sendStr(fd, "Point added.\n");
                } else {
                    sendStr(fd, "Bad format.\n");
                }
            } else if (cmd == "Removepoint") {
                std::string rest;
                std::getline(cmdin, rest);
                Point p;
                if (parsePoint(rest, p)) {
                    auto it = std::find_if(graph.begin(), graph.end(), [&](const Point& q) {
                        return q.x == p.x && q.y == p.y;
                    });
                    if (it != graph.end()) {
                        graph.erase(it);
                        sendStr(fd, "Point removed.\n");
                    } else {
                        sendStr(fd, "Point not found.\n");
                    }
                } else {
                    sendStr(fd, "Bad format.\n");
                }
            } else if (cmd == "CH") {
                auto hull = computeConvexHull(graph);
                double area = computeArea(hull);

                // Producer–Consumer check
                pthread_mutex_lock(&ch_mutex);
                if (area >= 100) {
                    wasAbove100 = true;
                    std::cout << "At least 100 units belong to CH\n";
                    pthread_cond_broadcast(&ch_cond);
                } else if (wasAbove100 && area < 100) {
                    std::cout << "At least 100 units no longer belong to CH\n";
                    pthread_cond_broadcast(&ch_cond);
                }
                pthread_mutex_unlock(&ch_mutex);

                std::ostringstream out;
                out << std::fixed << std::setprecision(6)
                    << "Convex hull area: " << area << "\n";
                sendStr(fd, out.str());
            } else {
                sendStr(fd, "Unknown command.\n" + helpText());
            }
        }
    }
}

int main() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9034);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(lfd);
        return 1;
    }

    if (bind(lfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(lfd);
        return 1;
    }
    if (listen(lfd, 10) < 0) {
        perror("listen");
        close(lfd);
        return 1;
    }

    std::cout << "Server (proactor) running on port 9034...\n";
    pthread_t tid = startProactor(lfd, handleClient);

    std::string line;
    while (true) {
        if (!std::getline(std::cin, line)) {
            std::cout << "EOF on stdin, shutting down server...\n";
            break;
        }
        if (line == "EXIT" || line == "exit") {
            std::cout << "Server EXIT command received, shutting down...\n";
            break;
        }
    }

    stopProactor(tid);
    close(lfd);
    {
        std::lock_guard<std::mutex> lock(graphMutex);
        for (auto &p : clientIds) {
            int fd = p.first;
            if (fd >= 0) {
                close(fd);
            }
        }
        clientIds.clear();
    }

    std::cout << "Server shutdown complete.\n";
    return 0;
}
