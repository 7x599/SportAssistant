#include "exercise/PushUpAnalyzer.h"

#include "exercise/Geometry.h"

#include <algorithm>
#include <array>
#include <utility>

namespace sport {

namespace {

struct SideIds {
    KeypointId shoulder;
    KeypointId elbow;
    KeypointId wrist;
    KeypointId hip;
    KeypointId ankle;
};

constexpr std::array<SideIds, 2> sides{{
    {KeypointId::LeftShoulder, KeypointId::LeftElbow, KeypointId::LeftWrist,
     KeypointId::LeftHip, KeypointId::LeftAnkle},
    {KeypointId::RightShoulder, KeypointId::RightElbow, KeypointId::RightWrist,
     KeypointId::RightHip, KeypointId::RightAnkle}
}};

} // namespace

PushUpAnalyzer::PushUpAnalyzer(AnalyzerConfig config)
    : config_(std::move(config)),
      smoother_(config_.smoothingAlpha, config_.confidenceThreshold) {}

std::optional<PushUpAnalyzer::SideAngles> PushUpAnalyzer::selectSide(const Pose& pose) const {
    std::optional<SideAngles> best;
    double bestConfidence = -1.0;
    for (const SideIds& side : sides) {
        const KeyPoint& shoulder = pose.at(side.shoulder);
        const KeyPoint& elbow = pose.at(side.elbow);
        const KeyPoint& wrist = pose.at(side.wrist);
        const KeyPoint& hip = pose.at(side.hip);
        const KeyPoint& ankle = pose.at(side.ankle);
        const auto elbowAngle = jointAngle(shoulder, elbow, wrist, config_.confidenceThreshold);
        const auto bodyAngle = jointAngle(shoulder, hip, ankle, config_.confidenceThreshold);
        if (!elbowAngle || !bodyAngle) {
            continue;
        }
        const double confidence = shoulder.confidence + elbow.confidence +
                                  wrist.confidence + hip.confidence + ankle.confidence;
        if (confidence > bestConfidence) {
            bestConfidence = confidence;
            best = SideAngles{*elbowAngle, *bodyAngle};
        }
    }
    return best;
}

const char* PushUpAnalyzer::phaseText(Phase phase) noexcept {
    switch (phase) {
    case Phase::Up: return "撑起";
    case Phase::Descending: return "下降";
    case Phase::Down: return "底部";
    case Phase::Ascending: return "上升";
    }
    return "未知";
}

ExerciseResult PushUpAnalyzer::makeResult(
    bool hasPose,
    double elbowAngle,
    double bodyAngle,
    bool eventOccurred,
    bool eventValid,
    std::string message) const {
    return ExerciseResult{
        ExerciseType::PushUp,
        validCount_,
        invalidCount_,
        elbowAngle,
        bodyAngle,
        hasPose,
        eventOccurred,
        eventValid,
        phaseText(phase_),
        message.empty() ? message_ : std::move(message)
    };
}

void PushUpAnalyzer::changePhase(Phase phase) noexcept {
    phase_ = phase;
    pendingPhase_ = phase;
    pendingFrames_ = 0;
}

ExerciseResult PushUpAnalyzer::update(const Pose& rawPose) {
    const Pose pose = smoother_.update(rawPose);
    const auto angles = selectSide(pose);
    if (!angles) {
        ++lostPoseFrames_;
        if (lostPoseFrames_ >= config_.lostPoseResetFrames) {
            changePhase(Phase::Up);
            reachedDown_ = false;
            smoother_.reset();
        }
        return makeResult(false, 0.0, 0.0, false, false, "关键点不完整，请让肩、肘、腕、髋、踝入镜");
    }

    lostPoseFrames_ = 0;
    const double angle = angles->elbow;
    const bool descending = angle < previousAngle_ - config_.directionEpsilonDegrees;
    const bool ascending = angle > previousAngle_ + config_.directionEpsilonDegrees;
    previousAngle_ = angle;
    minimumAngle_ = std::min(minimumAngle_, angle);
    minimumBodyAngle_ = std::min(minimumBodyAngle_, angles->bodyLine);

    auto stableTransition = [this](Phase target) {
        if (pendingPhase_ != target) {
            pendingPhase_ = target;
            pendingFrames_ = 1;
            if (config_.stableFrames <= 1) {
                changePhase(target);
                return true;
            }
            return false;
        }
        ++pendingFrames_;
        if (pendingFrames_ < std::max(1, config_.stableFrames)) {
            return false;
        }
        changePhase(target);
        return true;
    };
    auto cancelPending = [this] {
        pendingPhase_ = phase_;
        pendingFrames_ = 0;
    };

    bool eventOccurred = false;
    bool eventValid = false;

    switch (phase_) {
    case Phase::Up:
        message_ = angles->bodyLine < config_.pushUp.minimumBodyLineAngle
                       ? "请保持肩、髋、踝成一直线" : "准备完成";
        minimumAngle_ = angle;
        minimumBodyAngle_ = angles->bodyLine;
        if (angle < config_.pushUp.descentStartAngle && descending) {
            if (stableTransition(Phase::Descending)) {
                repStartTime_ = pose.timestampSeconds;
                reachedDown_ = false;
                minimumAngle_ = angle;
                minimumBodyAngle_ = angles->bodyLine;
            }
        } else {
            cancelPending();
        }
        break;

    case Phase::Descending:
        if (angle <= config_.pushUp.downAngle) {
            if (stableTransition(Phase::Down)) {
                reachedDown_ = true;
                message_ = "下压深度达标";
            }
        } else if (angle >= config_.pushUp.upAngle && ascending) {
            if (stableTransition(Phase::Up)) {
                ++invalidCount_;
                eventOccurred = true;
                message_ = "下压深度不足，本次不计数";
            }
        } else {
            cancelPending();
        }
        break;

    case Phase::Down:
        message_ = "底部稳定";
        if (ascending && angle > config_.pushUp.downAngle + 5.0) {
            stableTransition(Phase::Ascending);
        } else {
            cancelPending();
        }
        break;

    case Phase::Ascending:
        if (angle >= config_.pushUp.upAngle) {
            if (stableTransition(Phase::Up)) {
                const double duration = std::max(0.0, pose.timestampSeconds - repStartTime_);
                eventOccurred = true;
                eventValid = reachedDown_ && duration >= config_.minimumRepSeconds &&
                             duration <= config_.maximumRepSeconds &&
                             minimumBodyAngle_ >= config_.pushUp.minimumBodyLineAngle;
                if (eventValid) {
                    ++validCount_;
                    message_ = "动作标准";
                } else {
                    ++invalidCount_;
                    if (minimumBodyAngle_ < config_.pushUp.minimumBodyLineAngle) {
                        message_ = "身体弯曲明显，本次不计数";
                    } else if (duration < config_.minimumRepSeconds) {
                        message_ = "动作过快，本次不计数";
                    } else {
                        message_ = "动作周期异常，本次不计数";
                    }
                }
                reachedDown_ = false;
                minimumAngle_ = 180.0;
                minimumBodyAngle_ = 180.0;
            }
        } else if (descending && angle < config_.pushUp.downAngle + 8.0) {
            stableTransition(Phase::Down);
        } else {
            cancelPending();
        }
        break;
    }

    return makeResult(true, angle, angles->bodyLine, eventOccurred, eventValid);
}

void PushUpAnalyzer::reset() {
    smoother_.reset();
    phase_ = Phase::Up;
    pendingPhase_ = phase_;
    pendingFrames_ = 0;
    lostPoseFrames_ = 0;
    validCount_ = 0;
    invalidCount_ = 0;
    previousAngle_ = 180.0;
    repStartTime_ = 0.0;
    minimumAngle_ = 180.0;
    minimumBodyAngle_ = 180.0;
    reachedDown_ = false;
    message_ = "撑直后开始";
}

} // namespace sport
