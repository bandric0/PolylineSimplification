#include "efficientsimplifier.h"
#include <set>
#include <limits>

// Helper structure for the std::set to keep triangles sorted by area, then by index
struct TriangleNode {
    int index;
    double area;

    bool operator<(const TriangleNode& other) const {
        // Epsilon used for floating point comparison precision
        if (std::abs(area - other.area) > 1e-9) {
            return area < other.area;
        }
        // Tie-breaker: smaller original index
        return index < other.index;
    }
};

std::vector<SimplificationStep> EfficientSimplifier::simplify(const std::vector<QPointF>& polyline, int m) {
    std::vector<SimplificationStep> history;
    int n = polyline.size();

    // If the polyline already has equal or fewer points than requested
    if (n <= m) return history;

    // Simulated doubly-linked list to keep track of remaining neighbors in O(1) time
    std::vector<int> prev_neighbor(n);
    std::vector<int> next_neighbor(n);

    // Array to store current areas so we know exactly what to erase from the set
    std::vector<double> current_area(n, std::numeric_limits<double>::max());

    // BST to fetch the minimum area triangle in O(log n) time
    std::set<TriangleNode> active_triangles;

    // 1. Initialization
    for (int i = 0; i < n; ++i) {
        prev_neighbor[i] = i - 1;
        next_neighbor[i] = i + 1;
    }

    for (int i = 1; i < n - 1; ++i) {
        current_area[i] = doubleTriangleArea(polyline[prev_neighbor[i]], polyline[i], polyline[next_neighbor[i]]);
        active_triangles.insert({i, current_area[i]});
    }

    int current_points = n;

    // 2. Simplification loop
    while (current_points > m && !active_triangles.empty()) {
        // Fetch and remove the triangle with the smallest area
        auto it = active_triangles.begin();
        TriangleNode min_node = *it;
        active_triangles.erase(it);

        int removed_idx = min_node.index;

        // Update the doubly-linked list bounds
        int left = prev_neighbor[removed_idx];
        int right = next_neighbor[removed_idx];

        if (left >= 0) next_neighbor[left] = right;
        if (right < n) prev_neighbor[right] = left;

        // Record the current state for animation/history
        SimplificationStep step;
        step.removed_index = removed_idx;
        step.removed_area = min_node.area;

        int curr = 0;
        while (curr < n) {
            step.active_points.push_back(curr);
            curr = next_neighbor[curr]; // Jump to the next active point
        }
        history.push_back(step);

        // 3. Recalculate areas for the affected neighbors and update the set
        if (left > 0) {
            active_triangles.erase({left, current_area[left]});
            current_area[left] = doubleTriangleArea(polyline[prev_neighbor[left]], polyline[left], polyline[next_neighbor[left]]);
            active_triangles.insert({left, current_area[left]});
        }

        if (right < n - 1) {
            active_triangles.erase({right, current_area[right]});
            current_area[right] = doubleTriangleArea(polyline[prev_neighbor[right]], polyline[right], polyline[next_neighbor[right]]);
            active_triangles.insert({right, current_area[right]});
        }

        current_points--;
    }

    return history;
}
