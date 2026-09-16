#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <vector>
#include <QPointF>
#include <cmath>

struct SimplificationStep {
    int removed_index;
    double removed_area;
    std::vector<int> active_points;
};

inline double doubleTriangleArea(const QPointF& a, const QPointF& b, const QPointF& c) {
    return std::abs((b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x()));
}

#endif // CORE_TYPES_H
