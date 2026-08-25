#include "vision/PoseDetector.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <array>
#include <exception>

namespace sport {

namespace {

constexpr std::array<std::pair<KeypointId, KeypointId>, 16> skeleton{{
    {KeypointId::LeftShoulder, KeypointId::RightShoulder},
    {KeypointId::LeftShoulder, KeypointId::LeftElbow},
    {KeypointId::LeftElbow, KeypointId::LeftWrist},
    {KeypointId::RightShoulder, KeypointId::RightElbow},
    {KeypointId::RightElbow, KeypointId::RightWrist},
    {KeypointId::LeftShoulder, KeypointId::LeftHip},
    {KeypointId::RightShoulder, KeypointId::RightHip},
    {KeypointId::LeftHip, KeypointId::RightHip},
    {KeypointId::LeftHip, KeypointId::LeftKnee},
    {KeypointId::LeftKnee, KeypointId::LeftAnkle},
    {KeypointId::RightHip, KeypointId::RightKnee},
    {KeypointId::RightKnee, KeypointId::RightAnkle},
    {KeypointId::Nose, KeypointId::LeftEye},
    {KeypointId::Nose, KeypointId::RightEye},
    {KeypointId::LeftEye, KeypointId::LeftEar},
    {KeypointId::RightEye, KeypointId::RightEar}
}};

} // namespace

PoseDetector::PoseDetector(const std::filesystem::path& modelPath) {
    load(modelPath);
}

bool PoseDetector::load(const std::filesystem::path& modelPath) {
    try {
        if (!std::filesystem::exists(modelPath)) {
            lastError_ = "姿态模型不存在: " + modelPath.string();
            loaded_ = false;
            return false;
        }
        net_ = cv::dnn::readNetFromONNX(modelPath.string());
        net_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        loaded_ = !net_.empty();
        lastError_ = loaded_ ? std::string{} : "ONNX 模型加载失败";
        return loaded_;
    } catch (const std::exception& exception) {
        loaded_ = false;
        lastError_ = exception.what();
        return false;
    }
}

Pose PoseDetector::detect(const cv::Mat& frame, double timestampSeconds) {
    Pose pose;
    pose.frameIndex = ++frameIndex_;
    pose.timestampSeconds = timestampSeconds;
    if (!loaded_ || frame.empty()) {
        return pose;
    }

    const float scale = std::min(
        static_cast<float>(inputWidth_) / static_cast<float>(frame.cols),
        static_cast<float>(inputHeight_) / static_cast<float>(frame.rows));
    const int resizedWidth = static_cast<int>(frame.cols * scale);
    const int resizedHeight = static_cast<int>(frame.rows * scale);
    const int padX = (inputWidth_ - resizedWidth) / 2;
    const int padY = (inputHeight_ - resizedHeight) / 2;

    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(resizedWidth, resizedHeight));
    cv::Mat letterboxed(inputHeight_, inputWidth_, CV_8UC3, cv::Scalar(114, 114, 114));
    resized.copyTo(letterboxed(cv::Rect(padX, padY, resizedWidth, resizedHeight)));

    cv::Mat blob = cv::dnn::blobFromImage(
        letterboxed, 1.0 / 255.0, cv::Size(inputWidth_, inputHeight_),
        cv::Scalar(), true, false);
    net_.setInput(blob);
    cv::Mat output = net_.forward();
    if (output.empty()) {
        return pose;
    }

    cv::Mat candidates;
    if (output.dims == 3) {
        const int first = output.size[1];
        const int second = output.size[2];
        if (first <= 128 && second > first) {
            cv::Mat shaped(first, second, CV_32F, output.ptr<float>());
            cv::transpose(shaped, candidates);
        } else {
            candidates = cv::Mat(first, second, CV_32F, output.ptr<float>()).clone();
        }
    } else if (output.dims == 2) {
        candidates = output;
    } else {
        lastError_ = "无法识别的模型输出维度";
        return pose;
    }

    if (candidates.cols < 5 + static_cast<int>(Pose::KeypointCount) * 3) {
        lastError_ = "模型输出不是 COCO 17 关键点格式";
        return pose;
    }

    int bestRow = -1;
    float bestScore = personThreshold_;
    for (int row = 0; row < candidates.rows; ++row) {
        const float score = candidates.at<float>(row, 4);
        if (score > bestScore) {
            bestScore = score;
            bestRow = row;
        }
    }
    if (bestRow < 0) {
        return pose;
    }

    const float* values = candidates.ptr<float>(bestRow);
    pose.score = bestScore;
    for (std::size_t index = 0; index < Pose::KeypointCount; ++index) {
        const std::size_t base = 5 + index * 3;
        KeyPoint& point = pose.keypoints[index];
        point.x = (values[base] - static_cast<float>(padX)) / scale;
        point.y = (values[base + 1] - static_cast<float>(padY)) / scale;
        point.x = std::clamp(point.x, 0.0F, static_cast<float>(frame.cols - 1));
        point.y = std::clamp(point.y, 0.0F, static_cast<float>(frame.rows - 1));
        point.confidence = values[base + 2];
    }
    return pose;
}

void PoseDetector::drawPose(cv::Mat& frame, const Pose& pose) const {
    const cv::Scalar lineColor(214, 222, 55);
    const cv::Scalar pointColor(196, 235, 88);
    for (const auto& [firstId, secondId] : skeleton) {
        const KeyPoint& first = pose.at(firstId);
        const KeyPoint& second = pose.at(secondId);
        if (!first.visible(keypointThreshold_) || !second.visible(keypointThreshold_)) {
            continue;
        }
        cv::line(frame,
                 cv::Point(static_cast<int>(first.x), static_cast<int>(first.y)),
                 cv::Point(static_cast<int>(second.x), static_cast<int>(second.y)),
                 lineColor, 2, cv::LINE_AA);
    }
    for (const KeyPoint& point : pose.keypoints) {
        if (point.visible(keypointThreshold_)) {
            cv::circle(frame,
                       cv::Point(static_cast<int>(point.x), static_cast<int>(point.y)),
                       4, pointColor, cv::FILLED, cv::LINE_AA);
        }
    }
}

} // namespace sport
