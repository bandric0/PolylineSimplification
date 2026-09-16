#ifndef EFFICIENTSIMPLIFIER_H
#define EFFICIENTSIMPLIFIER_H

#include <vector>
#include <QPointF>
#include "core_types.h"

class EfficientSimplifier {
public:
    static std::vector<SimplificationStep> simplify(const std::vector<QPointF>& polyline, int m);
};

#endif // EFFICIENTSIMPLIFIER_H
