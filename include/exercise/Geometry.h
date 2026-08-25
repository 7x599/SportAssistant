#pragma once

#include "common/Pose.h"

#include <optional>

namespace sport {

[[nodiscard]] std::optional<double> jointAngle(
    const KeyPoint& first,
    const KeyPoint& vertex,
    const KeyPoint& third,
    double confidenceThreshold);

} // namespace sport
