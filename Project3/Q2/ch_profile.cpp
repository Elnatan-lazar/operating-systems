#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iomanip>

struct Point {
    double x, y;
    bool operator<(const Point& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
};

// Computes the cross product of vectors OA and OB
double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Convex Hull using std::deque
std::vector<Point> convexHullWithDeque(const std::vector<Point>& points) {
    std::vector<Point> sorted = points;
    std::sort(sorted.begin(), sorted.end());
    std::deque<Point> hull;

    for (const Point& p : sorted) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0) {
            hull.pop_back();
        }
        hull.push_back(p);
    }

    size_t lower_size = hull.size();
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        while (hull.size() > lower_size && cross(hull[hull.size() - 2], hull[hull.size() - 1], *it) <= 0) {
            hull.pop_back();
        }
        hull.push_back(*it);
    }

    hull.pop_back(); // Remove duplicated point
    return std::vector<Point>(hull.begin(), hull.end());
}

// Convex Hull using std::list
std::vector<Point> convexHullWithList(const std::vector<Point>& points) {
    std::vector<Point> sorted = points;
    std::sort(sorted.begin(), sorted.end());
    std::list<Point> hull;

    for (const Point& p : sorted) {
        while (hull.size() >= 2) {
            auto last = std::prev(hull.end());
            auto second_last = std::prev(last);
            if (cross(*second_last, *last, p) <= 0) {
                hull.erase(last);
            } else break;
        }
        hull.push_back(p);
    }

    size_t lower_size = hull.size();
    for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
        while (hull.size() > lower_size) {
            auto last = std::prev(hull.end());
            auto second_last = std::prev(last);
            if (cross(*second_last, *last, *it) <= 0) {
                hull.erase(last);
            } else break;
        }
        hull.push_back(*it);
    }

    hull.pop_back(); // Remove duplicated point
    return std::vector<Point>(hull.begin(), hull.end());
}

// Generate random points for test
std::vector<Point> generatePoints(int n) {
    std::vector<Point> pts;
    for (int i = 0; i < n; ++i) {
        double x = rand() % 10000 / 100.0;
        double y = rand() % 10000 / 100.0;
        pts.push_back({x, y});
    }
    return pts;
}

int main() {
    std::ofstream out("timing_results.txt");
    const int NUM_POINTS = 100000;
    std::vector<Point> test_points = generatePoints(NUM_POINTS);

    auto start1 = std::chrono::high_resolution_clock::now();
    auto result1 = convexHullWithDeque(test_points);
    auto end1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed1 = end1 - start1;
    out << "Convex Hull with deque: " << elapsed1.count() << " seconds\n";

    auto start2 = std::chrono::high_resolution_clock::now();
    auto result2 = convexHullWithList(test_points);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = end2 - start2;
    out << "Convex Hull with list: " << elapsed2.count() << " seconds\n";

    out.close();
    std::cout << "✅ Timing results written to 'timing_results.txt'\n";
    return 0;
}
