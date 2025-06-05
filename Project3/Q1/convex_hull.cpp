#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>

// Represents a 2D point
struct Point {
    double x, y;

    // Lexicographic comparison: first by x, then by y
    bool operator<(const Point& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
};

// Computes the cross product of vectors OA and OB (O is the origin)
double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Computes the convex hull using Andrew's monotone chain algorithm
std::vector<Point> computeConvexHull(std::vector<Point>& points) {
    int n = points.size(), k = 0;
    if (n <= 3) return points;

    std::sort(points.begin(), points.end());
    std::vector<Point> hull(2 * n);

    // Build lower hull
    for (int i = 0; i < n; ++i) {
        while (k >= 2 && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
        hull[k++] = points[i];
    }

    // Build upper hull
    for (int i = n - 2, t = k + 1; i >= 0; --i) {
        while (k >= t && cross(hull[k - 2], hull[k - 1], points[i]) <= 0) k--;
        hull[k++] = points[i];
    }

    hull.resize(k - 1); // Remove duplicate last point
    return hull;
}

// Calculates the area of a polygon given by a sequence of points
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

int main() {
    int n;
    std::cout << "Welcome to the Convex Hull Area Calculator!\n";
    std::cout << "Please enter the number of points: ";

    while (!(std::cin >> n) || n <= 0) {
        std::cout << "Invalid number. Please enter a positive integer: ";
        std::cin.clear();                 // Clear error state
        std::cin.ignore(1000, '\n');      // Discard invalid input
    }

    std::vector<Point> points;
    std::cout << "Enter " << n << " points in format: x,y (e.g., 1.5,2.3)\n\n";

    int readCount = 0;
    while (readCount < n) {
        double x, y;
        char comma;

        std::cout << "Point " << (readCount + 1) << ": ";
        if (std::cin >> x >> comma >> y && comma == ',') {
            points.push_back({x, y});
            readCount++;
        } else {
            std::cout << "Invalid format. Please enter in format x,y (e.g., 3.2,4.1)\n";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
        }
    }

    std::vector<Point> convexHull = computeConvexHull(points);
    double area = computePolygonArea(convexHull);

    std::cout << "\n Area of the convex hull: "
              << std::fixed << std::setprecision(6)
              << area << std::endl;

    return 0;
}
