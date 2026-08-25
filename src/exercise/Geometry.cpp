#include "exercise/Geometry.h"

#include <algorithm>
#include <cmath>

namespace sport {

std::optional<double> jointAngle(
    const KeyPoint& first,
    const KeyPoint& vertex,
    const KeyPoint& third,
    double confidenceThreshold) {
    if (!first.visible(static_cast<float>(confidenceThreshold)) ||
        !vertex.visible(static_cast<float>(confidenceThreshold)) ||
        !third.visible(static_cast<float>(confidenceThreshold))) {
        return std::nullopt;
    }

    const double ax = static_cast<double>(first.x - vertex.x);
    const double ay = static_cast<double>(first.y - vertex.y);
    const double bx = static_cast<double>(third.x - vertex.x);
    const double by = static_cast<double>(third.y - vertex.y);
    const double aLength = std::hypot(ax, ay);
    const double bLength = std::hypot(bx, by);
    if (aLength < 1.0e-6 || bLength < 1.0e-6) {
        return std::nullopt;
    }

    const double cosine = std::clamp(
        (ax * bx + ay * by) / (aLength * bLength), -1.0, 1.0);
    constexpr double radiansToDegrees = 180.0 / 3.14159265358979323846;
    return std::acos(cosine) * radiansToDegrees;
}

} // namespace sport
