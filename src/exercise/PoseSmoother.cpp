#include "exercise/PoseSmoother.h"

#include <algorithm>

namespace sport {

PoseSmoother::PoseSmoother(double alpha, double confidenceThreshold)
    : alpha_(std::clamp(alpha, 0.0, 1.0)),
      confidenceThreshold_(confidenceThreshold) {}

Pose PoseSmoother::update(const Pose& rawPose) {
    if (!initialized_) {
        smoothed_ = rawPose;
        initialized_ = true;
        return smoothed_;
    }

    smoothed_.frameIndex = rawPose.frameIndex;
    smoothed_.timestampSeconds = rawPose.timestampSeconds;
    smoothed_.score = rawPose.score;

    for (std::size_t index = 0; index < Pose::KeypointCount; ++index) {
        const KeyPoint& raw = rawPose.keypoints[index];
        KeyPoint& filtered = smoothed_.keypoints[index];
        if (!raw.visible(static_cast<float>(confidenceThreshold_))) {
            filtered.confidence = raw.confidence;
            continue;
        }
        if (!filtered.visible(static_cast<float>(confidenceThreshold_))) {
            filtered = raw;
            continue;
        }
        filtered.x = static_cast<float>(alpha_ * raw.x + (1.0 - alpha_) * filtered.x);
        filtered.y = static_cast<float>(alpha_ * raw.y + (1.0 - alpha_) * filtered.y);
        filtered.confidence = raw.confidence;
    }
    return smoothed_;
}

void PoseSmoother::reset() noexcept {
    initialized_ = false;
    smoothed_ = {};
}

} // namespace sport
