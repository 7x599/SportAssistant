#include "exercise/SquatAnalyzer.h"

#include "exercise/Geometry.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace sport {

namespace {

struct SideIds {
    KeypointId shoulder;
    KeypointId hip;
    KeypointId knee;
    KeypointId ankle;
};

constexpr std::array<SideIds, 2> sides{{
    {KeypointId::LeftShoulder, KeypointId::LeftHip,
     KeypointId::LeftKnee, KeypointId::LeftAnkle},
    {KeypointId::RightShoulder, KeypointId::RightHip,
     KeypointId::RightKnee, KeypointId::RightAnkle}
}};

} // namespace

SquatAnalyzer::SquatAnalyzer(AnalyzerConfig config)
    : config_(std::move(config)),
      smoother_(config_.smoothingAlpha, config_.confidenceThreshold) {}

std::optional<SquatAnalyzer::SideAngles> SquatAnalyzer::selectSide(const Pose& pose) const {
    std::optional<SideAngles> best;
    double bestConfidence = -1.0;
    for (const SideIds& side : sides) {
        const KeyPoint& shoulder = pose.at(side.shoulder);
        const KeyPoint& hip = pose.at(side.hip);
        const KeyPoint& knee = pose.at(side.knee);
        const KeyPoint& ankle = pose.at(side.ankle);
        const auto kneeAngle = jointAngle(hip, knee, ankle, config_.confidenceThreshold);
        const auto torsoAngle = jointAngle(shoulder, hip, knee, config_.confidenceThreshold);
        if (!kneeAngle || !torsoAngle) {
            continue;
        }
        const double confidence = shoulder.confidence + hip.confidence +
                                  knee.confidence + ankle.confidence;
        if (confidence > bestConfidence) {
            bestConfidence = confidence;
            best = SideAngles{*kneeAngle, *torsoAngle};
        }
    }
    return best;
}

const char* SquatAnalyzer::phaseText(Phase phase) noexcept {
    switch (phase) {
    case Phase::Standing: return "站立";
    case Phase::Descending: return "下蹲";
    case Phase::Bottom: return "底部";
    case Phase::Ascending: return "上升";
    }
    return "未知";
}

ExerciseResult SquatAnalyzer::makeResult(
    bool hasPose,
    double kneeAngle,
    double torsoAngle,
    bool eventOccurred,
    bool eventValid,
    std::string message) const {
    return ExerciseResult{
        ExerciseType::Squat,
        validCount_,
        invalidCount_,
        kneeAngle,
        torsoAngle,
        hasPose,
        eventOccurred,
        eventValid,
        phaseText(phase_),
        message.empty() ? message_ : std::move(message)
    };
}

void SquatAnalyzer::changePhase(Phase phase) noexcept {
    phase_ = phase;
    pendingPhase_ = phase;
    pendingFrames_ = 0;
}

ExerciseResult SquatAnalyzer::update(const Pose& rawPose) {
    const Pose pose = smoother_.update(rawPose);
    const auto angles = selectSide(pose);
    if (!angles) {
        ++lostPoseFrames_;
        if (lostPoseFrames_ >= config_.lostPoseResetFrames) {
            changePhase(Phase::Standing);
            reachedBottom_ = false;
            minimumAngle_ = 180.0;
            smoother_.reset();
        }
        return makeResult(false, 0.0, 0.0, false, false, "关键点不完整，请侧身并露出全身");
    }

    lostPoseFrames_ = 0;
    const double angle = angles->knee;
    const bool descending = angle < previousAngle_ - config_.directionEpsilonDegrees;
    const bool ascending = angle > previousAngle_ + config_.directionEpsilonDegrees;
    previousAngle_ = angle;
    minimumAngle_ = std::min(minimumAngle_, angle);

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
    case Phase::Standing:
        message_ = "准备完成";
        minimumAngle_ = angle;
        if (angle < config_.squat.descentStartAngle && descending) {
            if (stableTransition(Phase::Descending)) {
                repStartTime_ = pose.timestampSeconds;
                minimumAngle_ = angle;
                reachedBottom_ = false;
                message_ = "保持膝盖稳定";
            }
        } else {
            cancelPending();
        }
        break;

    case Phase::Descending:
        if (angle <= config_.squat.bottomAngle) {
            if (stableTransition(Phase::Bottom)) {
                reachedBottom_ = true;
                message_ = "深度达标，开始上升";
            }
        } else if (angle >= config_.squat.standingAngle && ascending) {
            if (stableTransition(Phase::Standing)) {
                ++invalidCount_;
                eventOccurred = true;
                message_ = "下蹲深度不足，本次不计数";
            }
        } else {
            cancelPending();
        }
        break;

    case Phase::Bottom:
        message_ = "深度达标";
        if (ascending && angle > config_.squat.bottomAngle + 4.0) {
            stableTransition(Phase::Ascending);
        } else {
            cancelPending();
        }
        break;

    case Phase::Ascending:
        if (angle >= config_.squat.standingAngle) {
            if (stableTransition(Phase::Standing)) {
                const double duration = std::max(0.0, pose.timestampSeconds - repStartTime_);
                eventOccurred = true;
                eventValid = reachedBottom_ && duration >= config_.minimumRepSeconds &&
                             duration <= config_.maximumRepSeconds;
                if (eventValid) {
                    ++validCount_;
                    message_ = "动作标准";
                } else {
                    ++invalidCount_;
                    message_ = duration < config_.minimumRepSeconds
                                   ? "动作过快，本次不计数"
                                   : "动作周期异常，本次不计数";
                }
                reachedBottom_ = false;
                minimumAngle_ = 180.0;
            }
        } else if (descending && angle < config_.squat.bottomAngle + 8.0) {
            stableTransition(Phase::Bottom);
        } else {
            cancelPending();
        }
        break;
    }

    return makeResult(true, angle, angles->torso, eventOccurred, eventValid);
}

void SquatAnalyzer::reset() {
    smoother_.reset();
    phase_ = Phase::Standing;
    pendingPhase_ = phase_;
    pendingFrames_ = 0;
    lostPoseFrames_ = 0;
    validCount_ = 0;
    invalidCount_ = 0;
    previousAngle_ = 180.0;
    repStartTime_ = 0.0;
    minimumAngle_ = 180.0;
    reachedBottom_ = false;
    message_ = "站直后开始";
}

} // namespace sport
