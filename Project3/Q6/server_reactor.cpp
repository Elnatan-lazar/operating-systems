#include "../Q5/reactor.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

struct Point { double x, y; };
static std::vector<Point> graph;
static std::unordered_map<int, int> statePts;
static void* reactor = nullptr;

// דגלים להגנה על פעולות מקביליות
static bool isGraphBusy = false;
static int currentOpFd = -1;

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
    return "Commands:\nNewgraph n\nNewpoint x,y\nRemovepoint x,y\nCH\nEXIT\n";
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

void* clientHandler(int fd) {
    char buf[512];
    ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) {
        std::cout << "Client " << fd << " disconnected.\n";
        removeFdFromReactor(reactor, fd);
        close(fd);
        return nullptr;
    }

    buf[n] = 0;
    std::istringstream iss(buf);
    std::string line;

    while (std::getline(iss, line)) {
        std::cout << "Client " << fd << " sent command: " << line << std::endl;

        if (statePts[fd] > 0) {
            if (isGraphBusy && currentOpFd != fd) {
                sendStr(fd, "Another operation is in progress. Please wait.\n");
                continue;
            }
            isGraphBusy = true;
            currentOpFd = fd;

            Point p;
            if (parsePoint(line, p)) {
                graph.push_back(p);
                sendStr(fd, "Point added.\n");
                std::cout << "Point " << p.x << "," << p.y << " added to graph.\n";
                if (--statePts[fd] == 0)
                    sendStr(fd, "Graph input complete.\n");
            } else {
                sendStr(fd, "Bad format, use x,y\n");
            }

            isGraphBusy = false;
            currentOpFd = -1;
            continue;
        }

        std::istringstream cmdin(line);
        std::string cmd;
        cmdin >> cmd;

        if (cmd == "EXIT") {
            sendStr(fd, "Goodbye.\n");
            std::cout << "Client " << fd << " requested EXIT.\n";
            removeFdFromReactor(reactor, fd);
            close(fd);
            return nullptr;
        } else if (cmd == "Newgraph") {
            if (isGraphBusy && currentOpFd != fd) {
                sendStr(fd, "Another operation is in progress. Please wait.\n");
                continue;
            }
            isGraphBusy = true;
            currentOpFd = fd;

            int n;
            if (cmdin >> n) {
                graph.clear();
                statePts[fd] = n;
                sendStr(fd, "Insert " + std::to_string(n) + " points in x,y:\n");
                std::cout << "Client " << fd << " is creating new graph with " << n << " points.\n";
            } else {
                sendStr(fd, "Usage: Newgraph n\n");
            }

            isGraphBusy = false;
            currentOpFd = -1;
        } else if (cmd == "Newpoint") {
            if (isGraphBusy && currentOpFd != fd) {
                sendStr(fd, "Another operation is in progress. Please wait.\n");
                continue;
            }
            isGraphBusy = true;
            currentOpFd = fd;

            std::string rest;
            std::getline(cmdin, rest);
            Point p;
            if (parsePoint(rest, p)) {
                graph.push_back(p);
                sendStr(fd, "Point added.\n");
                std::cout << "Client " << fd << " added point " << p.x << "," << p.y << "\n";
            } else {
                sendStr(fd, "Bad format.\n");
            }

            isGraphBusy = false;
            currentOpFd = -1;
        } else if (cmd == "Removepoint") {
            if (isGraphBusy && currentOpFd != fd) {
                sendStr(fd, "Another operation is in progress. Please wait.\n");
                continue;
            }
            isGraphBusy = true;
            currentOpFd = fd;

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
                    std::cout << "Client " << fd << " removed point " << p.x << "," << p.y << "\n";
                } else {
                    sendStr(fd, "Point not found.\n");
                }
            } else {
                sendStr(fd, "Bad format.\n");
            }

            isGraphBusy = false;
            currentOpFd = -1;
        } else if (cmd == "CH") {
            if (isGraphBusy && currentOpFd != fd) {
                sendStr(fd, "Another operation is in progress. Please wait.\n");
                continue;
            }
            isGraphBusy = true;
            currentOpFd = fd;

            auto hull = computeConvexHull(graph);
            double area = computeArea(hull);
            std::ostringstream out;
            out << std::fixed << std::setprecision(6)
                << "Convex hull area: " << area << "\n";
            sendStr(fd, out.str());
            std::cout << "Client " << fd << " computed CH. Area = " << area << std::endl;

            isGraphBusy = false;
            currentOpFd = -1;
        } else {
            sendStr(fd, "Unknown command.\n" + helpText());
        }
    }

    return nullptr;
}

void* acceptHandler(int listenfd) {
    int client = accept(listenfd, nullptr, nullptr);
    if (client >= 0) {
        std::cout << "New client connected: fd " << client << std::endl;
        sendStr(client, helpText());
        statePts[client] = 0;
        addFdToReactor(reactor, client, clientHandler);
    }
    return nullptr;
}

int main() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9034);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(lfd, (sockaddr*)&addr, sizeof(addr));
    listen(lfd, 10);

    reactor = startReactor();
    addFdToReactor(reactor, lfd, acceptHandler);

    std::cout << "Server running on port 9034...\n";
    while (true) pause();
    return 0;
}
