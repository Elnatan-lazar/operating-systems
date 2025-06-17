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
#include <mutex>
#include <cstring>

// -- shared graph state ---------------------------------------------------
struct Point {
    double x, y;
    // Add equality operator for std::find and comparison for sorting
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

static std::vector<Point> graph;               // the single shared graph
static std::unordered_map<int,int> statePts;   // client_fd → points left to read

// Mutex to protect the shared graph and statePts data.
static std::mutex graph_mutex;

// -- utility functions -----------------------------------------------------

// clean whitespace/CR/LF and parse "x,y"
bool parsePoint(const std::string &in, Point &out) {
    std::string s;
    // Filter out spaces, carriage returns, and newlines
    for (char c : in) {
        if (c != ' ' && c != '\r' && c != '\n') {
            s += c;
        }
    }
    // If the string is empty after filtering, it's not a valid point
    if (s.empty()) return false;

    std::istringstream iss(s);
    double x,y; char comma;
    // Ensure that after parsing x, comma, y, the stream is at the end (no extra junk)
    if ((iss >> x >> comma >> y) && comma == ',' && iss.eof()) {
        out = {x,y}; return true;
    }
    return false;
}

// write string to socket
void sendStr(int fd, const std::string &msg) {
    send(fd, msg.c_str(), msg.size(), 0);
}

// help menu
std::string helpText() {
    return
            "Commands:\n"
            "  Newgraph n      – clear graph, then send n points\n"
            "  Newpoint x,y    – add a point\n"
            "  Removepoint x,y – remove a point\n"
            "  CH              – compute convex hull area\n"
            "  EXIT            – disconnect\n";
}

// convex-hull & area (Andrew + Shoelace)
static auto cross = [](Point O, Point A, Point B){
    return (A.x-O.x)*(B.y-O.y) - (A.y-O.y)*(B.x-O.x);
};

std::vector<Point> computeConvexHull(std::vector<Point> pts) {
    sort(pts.begin(), pts.end(),[](auto &a,auto &b){
        return a.x<b.x || (a.x==b.x && a.y<b.y);
    });

    std::vector<Point> hull;
    if (pts.size() <= 2) {
        return pts;
    }

    for (auto &p : pts) {
        while (hull.size() >= 2 && cross(hull[hull.size()-2], hull[hull.size()-1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    size_t lower_hull_size = hull.size();
    for (int i = int(pts.size()) - 2; i >= 0; --i) {
        while (hull.size() > lower_hull_size && cross(hull[hull.size()-2], hull[hull.size()-1], pts[i]) <= 0)
            hull.pop_back();
        hull.push_back(pts[i]);
    }

    if (!hull.empty()) {
        hull.pop_back();
    }
    return hull;
}

double computeArea(const std::vector<Point>& poly){
    double a=0;
    int n=poly.size();
    for(int i=0;i<n;i++){
        const auto &p1=poly[i], &p2=poly[(i+1)%n];
        a += p1.x*p2.y - p2.x*p1.y;
    }
    return std::abs(a)/2.0;
}

// --- Per-client state for persistent input buffer ---
// This will help in handling partial reads from recv
// and ensure we always process complete lines.
static std::unordered_map<int, std::string> client_input_buffers;

// -- callback for client sockets -------------------------------------------

void *clientHandler(int fd) {
    char buf[512]; // A buffer for incoming data
    memset(buf, 0, sizeof(buf));

    // Acquire mutex for shared data access
    std::lock_guard<std::mutex> lock(graph_mutex);

    ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0); // Receive data from client
    if (n <= 0) {
        // Client disconnected or error.
        std::cout << "Client " << fd << " disconnected or error. Bytes read: " << n << "\n";
        removeFdFromReactor(startReactor(), fd);
        close(fd);
        statePts.erase(fd); // Clean up client state
        client_input_buffers.erase(fd); // Clean up input buffer
        return nullptr;
    }
    buf[n] = '\0'; // Null-terminate the received data

    // Append new data to the client's existing input buffer
    client_input_buffers[fd] += buf;

    std::string &current_buffer = client_input_buffers[fd];
    std::istringstream iss(current_buffer);
    std::string line;

    // A temporary string to build the remaining buffer for the next iteration
    std::string remaining_buffer;

    // Process lines from the buffer
    while (std::getline(iss, line)) {
        // Check if the line is complete (ends with a newline)
        // If not, it means this is a partial line, put it back and break.
        if (iss.eof() && !current_buffer.empty() && current_buffer.back() != '\n' && current_buffer.back() != '\r') {
            // This is a partial line, store it for the next recv
            remaining_buffer = line;
            break;
        }

        // Clean the line from potential \r characters at the end
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // --- Process the command/point ---
        // If we're in point-reading mode:
        if (statePts.count(fd) && statePts[fd] > 0) {
            int needed = statePts[fd];
            // Log the received line for debugging
            std::cout << "Client " << fd << ": Expected point. Processing line: \"" << line << "\"\n";

            Point p;
            if (parsePoint(line, p)) {
                bool pointExists = false;
                for (const auto& existing_p : graph) {
                    if (existing_p.x == p.x && existing_p.y == p.y) {
                        pointExists = true;
                        break;
                    }
                }
                if (!pointExists) {
                    graph.push_back(p);
                    sendStr(fd, "Added: " + line + "\n");
                    std::cout << "Client " << fd << ": Added point " << line << "\n";
                    --needed;
                } else {
                    sendStr(fd, "Point already exists.\n");
                    std::cout << "Client " << fd << ": Point " << line << " already exists.\n";
                }
            } else {
                sendStr(fd, "Bad point format, use x,y\n");
                std::cout << "Client " << fd << ": Bad point format for line \"" << line << "\"\n";
            }
            statePts[fd] = needed; // Update points needed
            if (needed == 0) {
                sendStr(fd, "All points received.\n");
                std::cout << "Client " << fd << ": All points received. Returning to command mode.\n";
            }

        } else { // Normal command mode:
            std::istringstream cmd_iss(line);
            std::string cmd;
            cmd_iss >> cmd;

            std::cout << "Client " << fd << ": Received command: \"" << line << "\"\n";

            if (cmd == "EXIT" || cmd == "exit") {
                sendStr(fd, "Bye!\n");
                std::cout << "Client " << fd << " requested EXIT.\n";
                removeFdFromReactor(startReactor(), fd);
                close(fd);
                statePts.erase(fd);
                client_input_buffers.erase(fd); // Clean up input buffer
                return nullptr;
            }
            if (cmd == "Newgraph") {
                int cnt;
                cmd_iss >> cnt;
                if (cmd_iss.fail() || cnt < 0) { // Basic validation
                    sendStr(fd, "Invalid Newgraph command. Usage: Newgraph N (N >= 0)\n");
                    std::cout << "Client " << fd << ": Invalid Newgraph command: " << line << "\n";
                } else {
                    graph.clear();
                    statePts[fd] = cnt;
                    sendStr(fd, "Insert " + std::to_string(cnt) + " points:\n");
                    std::cout << "Client " << fd << ": Newgraph command. Expecting " << cnt << " points.\n";
                }
            }
            else if (cmd == "Newpoint") {
                std::string rest;
                std::getline(cmd_iss, rest);
                Point p;
                if (parsePoint(rest, p)) {
                    bool pointExists = false;
                    for (const auto& existing_p : graph) {
                        if (existing_p.x == p.x && existing_p.y == p.y) {
                            pointExists = true;
                            break;
                        }
                    }
                    if (!pointExists) {
                        graph.push_back(p);
                        sendStr(fd, "Added point.\n");
                        std::cout << "Client " << fd << ": Added new point: " << rest << "\n";
                    } else {
                        sendStr(fd, "Point already exists.\n");
                        std::cout << "Client " << fd << ": Point " << rest << " already exists.\n";
                    }
                } else {
                    sendStr(fd, "Bad format, use x,y\n");
                    std::cout << "Client " << fd << ": Bad format for Newpoint: \"" << rest << "\"\n";
                }
            }
            else if (cmd == "Removepoint") {
                std::string rest;
                std::getline(cmd_iss, rest);
                Point p;
                if (parsePoint(rest, p)) {
                    auto it = std::find_if(graph.begin(),graph.end(),
                                           [&](auto &q){return q.x==p.x && q.y==p.y;});
                    if (it != graph.end()){
                        graph.erase(it);
                        sendStr(fd, "Removed.\n");
                        std::cout << "Client " << fd << ": Removed point: " << rest << "\n";
                    } else {
                        sendStr(fd, "Point not found.\n");
                        std::cout << "Client " << fd << ": Point not found for removal: " << rest << "\n";
                    }
                } else {
                    sendStr(fd, "Bad format, use x,y\n");
                    std::cout << "Client " << fd << ": Bad format for Removepoint: \"" << rest << "\"\n";
                }
            }
            else if (cmd == "CH") {
                auto hull = computeConvexHull(graph);
                double area = computeArea(hull);
                std::ostringstream os;
                os << std::fixed << std::setprecision(6)
                   << "Hull area: " << area << "\n";
                sendStr(fd, os.str());
                std::cout << "Client " << fd << ": Computed CH. Area: " << area << ". Current graph size: " << graph.size() << "\n";
            }
            else {
                sendStr(fd, "Unknown cmd.\n" + helpText());
                std::cout << "Client " << fd << ": Unknown command: \"" << line << "\"\n";
            }
        }
    }
    // Update the buffer with any remaining partial line
    current_buffer = remaining_buffer;
    return nullptr;
}

// -- callback for listening socket ----------------------------------------

void *acceptHandler(int listenfd) {
    int client = accept(listenfd,nullptr,nullptr);
    if (client > 0) {
        // Acquire mutex for shared data access (statePts)
        std::lock_guard<std::mutex> lock(graph_mutex);
        std::cout << "New client connected. FD: " << client << std::endl;
        sendStr(client, helpText());
        statePts[client] = 0; // Initialize state for new client
        client_input_buffers[client] = ""; // Initialize empty input buffer for new client
        addFdToReactor(startReactor(), client, clientHandler); // Add client to reactor for handling
    }
    return nullptr;
}

// -- main ------------------------------------------------------------------

int main(){
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if (lfd < 0) {
        perror("socket failed");
        return 1;
    }

    // Allow immediate reuse of the port
    int optval = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt failed");
        close(lfd);
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9034);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(lfd,(sockaddr*)&addr,sizeof(addr)) < 0) {
        perror("bind failed");
        close(lfd);
        return 1;
    }
    if (listen(lfd,5) < 0) {
        perror("listen failed");
        close(lfd);
        return 1;
    }

    void *reactor = startReactor();
    addFdToReactor(reactor, lfd, acceptHandler);

    std::cout << "Reactor server up on port 9034\n";
    while (true) {
        pause();
    }

    stopReactor(reactor);
    close(lfd);
    return 0;
}