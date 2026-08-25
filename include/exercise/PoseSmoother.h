#pragma once

#include "common/Pose.h"

#include <array>

namespace sport {

class PoseSmoother {
public:
    explicit PoseSmoother(double alpha = 0.42, double confidenceThreshold = 0.35);

    [[nodiscard]] Pose update(const Pose& rawPose);
    void reset() noexcept;

private:
    double alpha_;
    double confidenceThreshold_;
    bool initialized_{false};
    Pose smoothed_{};
};

} // namespace sport
