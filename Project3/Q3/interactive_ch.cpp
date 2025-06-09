#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

// Represents a 2D point with comparison and equality operators
struct Point {
    double x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator<(const Point& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
};

// Calculates the cross product of vectors OA and OB
double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Computes the convex hull using Andrew's monotone chain algorithm
std::vector<Point> computeConvexHull(std::vector<Point> points) {
    std::sort(points.begin(), points.end());
    std::vector<Point> hull;

    // Lower hull
    for (const auto& p : points) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size()-2], hull[hull.size()-1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    // Upper hull
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

// Calculates the area of a polygon
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

// Parses a string of format "x,y" into a Point, returns false if invalid
bool parsePoint(const std::string& input, Point& outPoint) {
    std::istringstream iss(input);
    double x, y;
    char comma;
    if ((iss >> x >> comma >> y) && comma == ',' && iss.eof()) {
        outPoint = {x, y};
        return true;
    }
    return false;
}

int main() {
    std::vector<Point> graph;
    std::string line;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "Newgraph") {
            int n;
            iss >> n;
            graph.clear();
            std::cout << "Insert " << n << " points in format x,y:" << std::endl;

            int added = 0;
            while (added < n) {
                std::string pointLine;
                std::getline(std::cin, pointLine);
                Point p;
                if (parsePoint(pointLine, p)) {
                    if (std::find(graph.begin(), graph.end(), p) == graph.end()) {
                        graph.push_back(p);
                        std::cout << "Point " << p.x << "," << p.y << " added to graph." << std::endl;
                        ++added;
                    } else {
                        std::cout << "Point already exists in graph." << std::endl;
                    }
                } else {
                    std::cout << "Invalid format. Please enter point as x,y" << std::endl;
                }
            }

        } else if (cmd == "CH") {
            std::vector<Point> hull = computeConvexHull(graph);
            double area = computePolygonArea(hull);
            std::cout << "area of the convex hull " << std::fixed << std::setprecision(6) << area << std::endl;

        } else if (cmd == "Newpoint") {
            std::string rest;
            std::getline(iss, rest);
            Point p;
            if (parsePoint(rest, p)) {
                if (std::find(graph.begin(), graph.end(), p) == graph.end()) {
                    graph.push_back(p);
                    std::cout << "Point " << p.x << "," << p.y << " added to graph." << std::endl;
                } else {
                    std::cout << "Point already exists in graph." << std::endl;
                }
            } else {
                std::cout << "Invalid format. Please enter point as x,y" << std::endl;
            }

        } else if (cmd == "Removepoint") {
            std::string rest;
            std::getline(iss, rest);
            Point p;
            if (parsePoint(rest, p)) {
                auto it = std::find(graph.begin(), graph.end(), p);
                if (it != graph.end()) {
                    graph.erase(it);
                    std::cout << "Point " << p.x << "," << p.y << " removed from graph." << std::endl;
                } else {
                    std::cout << "Point not found in graph." << std::endl;
                }
            } else {
                std::cout << "Invalid format. Please enter point as x,y" << std::endl;
            }

        } else {
            std::cout << "Unknown command. Valid commands: Newgraph, CH, Newpoint, Removepoint" << std::endl;
        }
    }

    return 0;
}
