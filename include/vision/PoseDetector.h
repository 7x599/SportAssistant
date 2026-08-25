#pragma once

#include "common/Pose.h"

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

#include <filesystem>
#include <string>

namespace sport {

class PoseDetector {
public:
    PoseDetector() = default;
    explicit PoseDetector(const std::filesystem::path& modelPath);

    [[nodiscard]] bool load(const std::filesystem::path& modelPath);
    [[nodiscard]] bool isLoaded() const noexcept { return loaded_; }
    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }
    [[nodiscard]] Pose detect(const cv::Mat& frame, double timestampSeconds);
    void drawPose(cv::Mat& frame, const Pose& pose) const;

private:
    cv::dnn::Net net_;
    bool loaded_{false};
    std::string lastError_;
    std::uint64_t frameIndex_{0};
    int inputWidth_{640};
    int inputHeight_{640};
    float personThreshold_{0.35F};
    float keypointThreshold_{0.35F};
};

} // namespace sport
