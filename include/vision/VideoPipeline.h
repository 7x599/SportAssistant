#pragma once

#include "common/ExerciseType.h"
#include "common/Pose.h"
#include "vision/DemoPoseSource.h"
#include "vision/PoseDetector.h"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <chrono>
#include <filesystem>
#include <string>

namespace sport {

enum class InputMode { Camera, File, Demo };

struct ProcessedFrame {
    cv::Mat image;
    Pose pose;
    bool hasFrame{false};
    bool hasPose{false};
    double fps{0.0};
};

class VideoPipeline {
public:
    [[nodiscard]] bool loadModel(const std::filesystem::path& modelPath);
    [[nodiscard]] bool openCamera(int index = 0);
    [[nodiscard]] bool openFile(const std::filesystem::path& path);
    void openDemo(ExerciseType exercise);
    void setExercise(ExerciseType exercise);
    [[nodiscard]] ProcessedFrame read();
    void close();

    [[nodiscard]] InputMode mode() const noexcept { return mode_; }
    [[nodiscard]] const std::string& lastError() const noexcept { return lastError_; }
    [[nodiscard]] bool modelLoaded() const noexcept { return detector_.isLoaded(); }

private:
    using Clock = std::chrono::steady_clock;

    cv::VideoCapture capture_;
    PoseDetector detector_;
    DemoPoseSource demo_;
    InputMode mode_{InputMode::Demo};
    ExerciseType exercise_{ExerciseType::Squat};
    std::string lastError_;
    Clock::time_point started_{Clock::now()};
    Clock::time_point previousFrame_{Clock::now()};
};

} // namespace sport
