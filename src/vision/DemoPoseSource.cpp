#include "vision/DemoPoseSource.h"

#include "exercise/Geometry.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace sport {

namespace {

constexpr double pi = 3.14159265358979323846;

KeyPoint point(double x, double y) {
    return KeyPoint{static_cast<float>(x), static_cast<float>(y), 0.98F};
}

double cycleAngle(double seconds, double top, double bottom) {
    constexpr double cycle = 3.2;
    const double phase = std::fmod(std::max(0.0, seconds), cycle) / cycle;
    if (phase < 0.18) return top;
    if (phase < 0.46) {
        const double t = (phase - 0.18) / 0.28;
        return top + (bottom - top) * t;
    }
    if (phase < 0.60) return bottom;
    if (phase < 0.90) {
        const double t = (phase - 0.60) / 0.30;
        return bottom + (top - bottom) * t;
    }
    return top;
}

constexpr std::array<std::pair<KeypointId, KeypointId>, 12> demoSkeleton{{
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
    {KeypointId::RightKnee, KeypointId::RightAnkle}
}};

} // namespace

DemoPoseSource::DemoPoseSource() : started_(Clock::now()) {}

void DemoPoseSource::reset(ExerciseType exercise) {
    exercise_ = exercise;
    started_ = Clock::now();
    frameIndex_ = 0;
}

Pose DemoPoseSource::makeSquatPose(double angle, double timestamp) const {
    Pose pose;
    pose.score = 0.99F;
    pose.timestampSeconds = timestamp;
    pose.frameIndex = frameIndex_;

    const double kneeX = 520.0;
    const double kneeY = 440.0;
    const double thigh = 165.0;
    const double shin = 175.0;
    const double crouch = (170.0 - angle) / 85.0;
    const double hipX = kneeX - 20.0 - crouch * 125.0;
    const double hipY = kneeY - std::sqrt(std::max(1.0, thigh * thigh -
                                                   (kneeX - hipX) * (kneeX - hipX)));
    const double v1 = std::atan2(hipY - kneeY, hipX - kneeX);
    const double ankleDirection = v1 + angle * pi / 180.0;
    const double ankleX = kneeX + shin * std::cos(ankleDirection);
    const double ankleY = kneeY + shin * std::sin(ankleDirection);
    const double shoulderX = hipX - 35.0 * crouch;
    const double shoulderY = hipY - 190.0;

    for (int side = 0; side < 2; ++side) {
        const double offset = side == 0 ? -12.0 : 12.0;
        const KeypointId shoulder = side == 0 ? KeypointId::LeftShoulder : KeypointId::RightShoulder;
        const KeypointId elbow = side == 0 ? KeypointId::LeftElbow : KeypointId::RightElbow;
        const KeypointId wrist = side == 0 ? KeypointId::LeftWrist : KeypointId::RightWrist;
        const KeypointId hip = side == 0 ? KeypointId::LeftHip : KeypointId::RightHip;
        const KeypointId knee = side == 0 ? KeypointId::LeftKnee : KeypointId::RightKnee;
        const KeypointId ankle = side == 0 ? KeypointId::LeftAnkle : KeypointId::RightAnkle;
        pose.at(shoulder) = point(shoulderX, shoulderY + offset);
        pose.at(elbow) = point(shoulderX + 95.0, shoulderY + 80.0 + offset);
        pose.at(wrist) = point(shoulderX + 165.0, shoulderY + 55.0 + offset);
        pose.at(hip) = point(hipX, hipY + offset);
        pose.at(knee) = point(kneeX, kneeY + offset);
        pose.at(ankle) = point(ankleX, ankleY + offset);
    }
    pose.at(KeypointId::Nose) = point(shoulderX + 18.0, shoulderY - 62.0);
    return pose;
}

Pose DemoPoseSource::makePushUpPose(double angle, double bodyAngle, double timestamp) const {
    Pose pose;
    pose.score = 0.99F;
    pose.timestampSeconds = timestamp;
    pose.frameIndex = frameIndex_;

    const double shoulderX = 285.0;
    const double shoulderY = 315.0;
    const double elbowX = 430.0;
    const double elbowY = shoulderY;
    const double forearmDirection = pi - angle * pi / 180.0;
    const double wristX = elbowX + 150.0 * std::cos(forearmDirection);
    const double wristY = elbowY + 150.0 * std::sin(forearmDirection);
    const double sag = (180.0 - bodyAngle) * 1.4;
    const double hipX = 535.0;
    const double hipY = shoulderY + sag;
    const double ankleX = 755.0;
    const double ankleY = shoulderY;

    for (int side = 0; side < 2; ++side) {
        const double offset = side == 0 ? -12.0 : 12.0;
        const KeypointId shoulder = side == 0 ? KeypointId::LeftShoulder : KeypointId::RightShoulder;
        const KeypointId elbow = side == 0 ? KeypointId::LeftElbow : KeypointId::RightElbow;
        const KeypointId wrist = side == 0 ? KeypointId::LeftWrist : KeypointId::RightWrist;
        const KeypointId hip = side == 0 ? KeypointId::LeftHip : KeypointId::RightHip;
        const KeypointId knee = side == 0 ? KeypointId::LeftKnee : KeypointId::RightKnee;
        const KeypointId ankle = side == 0 ? KeypointId::LeftAnkle : KeypointId::RightAnkle;
        pose.at(shoulder) = point(shoulderX, shoulderY + offset);
        pose.at(elbow) = point(elbowX, elbowY + offset);
        pose.at(wrist) = point(wristX, wristY + offset);
        pose.at(hip) = point(hipX, hipY + offset);
        pose.at(knee) = point((hipX + ankleX) * 0.5, (hipY + ankleY) * 0.5 + offset);
        pose.at(ankle) = point(ankleX, ankleY + offset);
    }
    pose.at(KeypointId::Nose) = point(shoulderX - 55.0, shoulderY - 28.0);
    return pose;
}

Pose DemoPoseSource::next(cv::Mat& frame) {
    const std::chrono::duration<double> elapsed = Clock::now() - started_;
    const double seconds = elapsed.count();
    ++frameIndex_;
    double mainAngle = 0.0;
    Pose pose;
    if (exercise_ == ExerciseType::Squat) {
        mainAngle = cycleAngle(seconds, 170.0, 82.0);
        pose = makeSquatPose(mainAngle, seconds);
    } else {
        mainAngle = cycleAngle(seconds, 170.0, 78.0);
        pose = makePushUpPose(mainAngle, 176.0, seconds);
    }
    draw(frame, pose, mainAngle);
    return pose;
}

void DemoPoseSource::draw(cv::Mat& frame, const Pose& pose, double mainAngle) const {
    frame = cv::Mat(720, 960, CV_8UC3, cv::Scalar(17, 23, 25));
    for (int x = 0; x < frame.cols; x += 80) {
        cv::line(frame, cv::Point(x, 0), cv::Point(x, frame.rows),
                 cv::Scalar(27, 38, 39), 1);
    }
    for (int y = 0; y < frame.rows; y += 80) {
        cv::line(frame, cv::Point(0, y), cv::Point(frame.cols, y),
                 cv::Scalar(27, 38, 39), 1);
    }
    for (const auto& [firstId, secondId] : demoSkeleton) {
        const KeyPoint& first = pose.at(firstId);
        const KeyPoint& second = pose.at(secondId);
        if (!first.visible() || !second.visible()) continue;
        cv::line(frame, cv::Point(static_cast<int>(first.x), static_cast<int>(first.y)),
                 cv::Point(static_cast<int>(second.x), static_cast<int>(second.y)),
                 cv::Scalar(221, 215, 54), 4, cv::LINE_AA);
    }
    for (const KeyPoint& keypoint : pose.keypoints) {
        if (!keypoint.visible()) continue;
        cv::circle(frame, cv::Point(static_cast<int>(keypoint.x), static_cast<int>(keypoint.y)),
                   7, cv::Scalar(197, 234, 83), cv::FILLED, cv::LINE_AA);
        cv::circle(frame, cv::Point(static_cast<int>(keypoint.x), static_cast<int>(keypoint.y)),
                   11, cv::Scalar(86, 91, 42), 2, cv::LINE_AA);
    }
    cv::putText(frame, exercise_ == ExerciseType::Squat ? "SQUAT DEMO" : "PUSH-UP DEMO",
                cv::Point(34, 58), cv::FONT_HERSHEY_DUPLEX, 1.0,
                cv::Scalar(220, 226, 222), 2, cv::LINE_AA);
    cv::putText(frame, std::to_string(static_cast<int>(std::lround(mainAngle))) + " deg",
                cv::Point(34, 102), cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(221, 215, 54), 2, cv::LINE_AA);
}

} // namespace sport
