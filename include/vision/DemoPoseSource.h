#pragma once

#include "common/ExerciseType.h"
#include "common/Pose.h"

#include <opencv2/core.hpp>

#include <chrono>

namespace sport {

class DemoPoseSource {
public:
    DemoPoseSource();
    void reset(ExerciseType exercise);
    [[nodiscard]] Pose next(cv::Mat& frame);

private:
    using Clock = std::chrono::steady_clock;

    [[nodiscard]] Pose makeSquatPose(double angle, double timestamp) const;
    [[nodiscard]] Pose makePushUpPose(double angle, double bodyAngle, double timestamp) const;
    void draw(cv::Mat& frame, const Pose& pose, double mainAngle) const;

    ExerciseType exercise_{ExerciseType::Squat};
    Clock::time_point started_{};
    std::uint64_t frameIndex_{0};
};

} // namespace sport
