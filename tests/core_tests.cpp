#include "exercise/Geometry.h"
#include "exercise/PushUpAnalyzer.h"
#include "exercise/SquatAnalyzer.h"
#include "training/TrainingSession.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

using sport::KeyPoint;
using sport::KeypointId;
using sport::Pose;

int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

KeyPoint point(double x, double y) {
    return KeyPoint{static_cast<float>(x), static_cast<float>(y), 0.99F};
}

Pose squatPose(double kneeAngle, double timestamp) {
    Pose pose;
    pose.score = 0.99F;
    pose.timestampSeconds = timestamp;
    const double radians = kneeAngle * 3.14159265358979323846 / 180.0;
    for (int side = 0; side < 2; ++side) {
        const double offset = side * 0.1;
        const auto shoulder = side == 0 ? KeypointId::LeftShoulder : KeypointId::RightShoulder;
        const auto hip = side == 0 ? KeypointId::LeftHip : KeypointId::RightHip;
        const auto knee = side == 0 ? KeypointId::LeftKnee : KeypointId::RightKnee;
        const auto ankle = side == 0 ? KeypointId::LeftAnkle : KeypointId::RightAnkle;
        const double hipX = 0.0;
        const double hipY = -1.0 + offset;
        pose.at(hip) = point(hipX, hipY);
        pose.at(knee) = point(0.0, offset);
        pose.at(ankle) = point(std::cos(-1.5707963267948966 + radians),
                               std::sin(-1.5707963267948966 + radians) + offset);
        pose.at(shoulder) = point(0.0, hipY - 1.0);
    }
    return pose;
}

Pose pushUpPose(double elbowAngle, double bodyAngle, double timestamp) {
    Pose pose;
    pose.score = 0.99F;
    pose.timestampSeconds = timestamp;
    const double elbowRadians = elbowAngle * 3.14159265358979323846 / 180.0;
    const double halfAngleRadians = bodyAngle * 3.14159265358979323846 / 360.0;
    const double sag = 2.0 / std::tan(halfAngleRadians);
    for (int side = 0; side < 2; ++side) {
        const double offset = side * 0.1;
        const auto shoulder = side == 0 ? KeypointId::LeftShoulder : KeypointId::RightShoulder;
        const auto elbow = side == 0 ? KeypointId::LeftElbow : KeypointId::RightElbow;
        const auto wrist = side == 0 ? KeypointId::LeftWrist : KeypointId::RightWrist;
        const auto hip = side == 0 ? KeypointId::LeftHip : KeypointId::RightHip;
        const auto ankle = side == 0 ? KeypointId::LeftAnkle : KeypointId::RightAnkle;
        pose.at(shoulder) = point(0.0, offset);
        pose.at(elbow) = point(1.0, offset);
        pose.at(wrist) = point(1.0 + std::cos(3.14159265358979323846 - elbowRadians),
                               std::sin(3.14159265358979323846 - elbowRadians) + offset);
        pose.at(hip) = point(2.0, sag + offset);
        pose.at(ankle) = point(4.0, offset);
    }
    return pose;
}

sport::AnalyzerConfig testConfig() {
    sport::AnalyzerConfig config;
    config.smoothingAlpha = 1.0;
    config.stableFrames = 1;
    config.directionEpsilonDegrees = 0.5;
    config.minimumRepSeconds = 0.5;
    return config;
}

void geometryTest() {
    const auto angle = sport::jointAngle(point(0.0, 1.0), point(0.0, 0.0),
                                         point(1.0, 0.0), 0.35);
    check(angle.has_value(), "right angle is available");
    check(angle && std::abs(*angle - 90.0) < 1.0e-6, "right angle equals 90 degrees");
}

void squatValidRepTest() {
    sport::SquatAnalyzer analyzer(testConfig());
    static_cast<void>(analyzer.update(squatPose(170.0, 0.0)));
    static_cast<void>(analyzer.update(squatPose(140.0, 0.2)));
    static_cast<void>(analyzer.update(squatPose(112.0, 0.5)));
    static_cast<void>(analyzer.update(squatPose(85.0, 0.8)));
    static_cast<void>(analyzer.update(squatPose(112.0, 1.0)));
    static_cast<void>(analyzer.update(squatPose(135.0, 1.2)));
    const auto result = analyzer.update(squatPose(165.0, 1.5));
    check(result.eventOccurred, "squat completes one event");
    check(result.eventValid, "full squat is valid");
    check(result.validCount == 1 && result.invalidCount == 0, "full squat increments valid count only");
}

void squatPartialRepTest() {
    sport::SquatAnalyzer analyzer(testConfig());
    static_cast<void>(analyzer.update(squatPose(170.0, 0.0)));
    static_cast<void>(analyzer.update(squatPose(140.0, 0.2)));
    static_cast<void>(analyzer.update(squatPose(125.0, 0.5)));
    const auto result = analyzer.update(squatPose(165.0, 1.0));
    check(result.eventOccurred && !result.eventValid, "partial squat emits invalid event");
    check(result.validCount == 0 && result.invalidCount == 1, "partial squat never increments valid count");
}

void pushUpValidRepTest() {
    sport::PushUpAnalyzer analyzer(testConfig());
    static_cast<void>(analyzer.update(pushUpPose(170.0, 178.0, 0.0)));
    static_cast<void>(analyzer.update(pushUpPose(140.0, 178.0, 0.2)));
    static_cast<void>(analyzer.update(pushUpPose(110.0, 178.0, 0.5)));
    static_cast<void>(analyzer.update(pushUpPose(82.0, 178.0, 0.8)));
    static_cast<void>(analyzer.update(pushUpPose(110.0, 178.0, 1.0)));
    static_cast<void>(analyzer.update(pushUpPose(140.0, 178.0, 1.2)));
    const auto result = analyzer.update(pushUpPose(165.0, 178.0, 1.5));
    check(result.eventOccurred && result.eventValid, "full push-up is valid");
    check(result.validCount == 1, "full push-up increments valid count");
}

void pushUpBentBodyTest() {
    sport::PushUpAnalyzer analyzer(testConfig());
    static_cast<void>(analyzer.update(pushUpPose(170.0, 178.0, 0.0)));
    static_cast<void>(analyzer.update(pushUpPose(140.0, 145.0, 0.2)));
    static_cast<void>(analyzer.update(pushUpPose(82.0, 145.0, 0.8)));
    static_cast<void>(analyzer.update(pushUpPose(110.0, 145.0, 1.0)));
    const auto result = analyzer.update(pushUpPose(165.0, 178.0, 1.5));
    check(result.eventOccurred && !result.eventValid, "bent-body push-up is rejected");
    check(result.invalidCount == 1, "bent-body push-up increments invalid count");
}

void missingPoseTest() {
    sport::SquatAnalyzer analyzer(testConfig());
    Pose missing;
    const auto result = analyzer.update(missing);
    check(!result.hasPose && !result.eventOccurred, "missing pose never creates a count event");
}

void trainingSessionTest() {
    sport::TrainingSession session;
    session.start(sport::ExerciseType::Squat, 5);
    sport::ExerciseResult result;
    result.exercise = sport::ExerciseType::Squat;
    result.validCount = 2;
    result.invalidCount = 1;
    session.update(result);
    check(session.validCount() == 2 && session.invalidCount() == 1,
          "training session mirrors analyzer counts");
    check(std::abs(session.completionRate() - 0.4) < 1.0e-9,
          "training session computes capped completion rate");
    const auto summary = session.finish();
    check(summary.targetCount == 5 && summary.validCount == 2,
          "training summary preserves target and counts");
}

} // namespace

int main() {
    geometryTest();
    squatValidRepTest();
    squatPartialRepTest();
    pushUpValidRepTest();
    pushUpBentBodyTest();
    missingPoseTest();
    trainingSessionTest();
    if (failures == 0) {
        std::cout << "All SportAssistant core tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
