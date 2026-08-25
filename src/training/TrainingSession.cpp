#include "training/TrainingSession.h"

#include <algorithm>

namespace sport {

void TrainingSession::start(ExerciseType exercise, int targetCount) {
    exercise_ = exercise;
    targetCount_ = std::max(1, targetCount);
    validCount_ = 0;
    invalidCount_ = 0;
    accumulatedSeconds_ = 0.0;
    runStarted_ = Clock::now();
    state_ = SessionState::Running;
}

void TrainingSession::pause() {
    if (state_ != SessionState::Running) {
        return;
    }
    accumulatedSeconds_ = activeSeconds();
    state_ = SessionState::Paused;
}

void TrainingSession::resume() {
    if (state_ != SessionState::Paused) {
        return;
    }
    runStarted_ = Clock::now();
    state_ = SessionState::Running;
}

SessionSummary TrainingSession::finish() {
    if (state_ == SessionState::Running) {
        accumulatedSeconds_ = activeSeconds();
    }
    state_ = SessionState::Finished;
    return summary();
}

void TrainingSession::update(const ExerciseResult& result) {
    if (state_ != SessionState::Running || result.exercise != exercise_) {
        return;
    }
    validCount_ = std::max(validCount_, result.validCount);
    invalidCount_ = std::max(invalidCount_, result.invalidCount);
}

void TrainingSession::reset() noexcept {
    state_ = SessionState::Idle;
    validCount_ = 0;
    invalidCount_ = 0;
    accumulatedSeconds_ = 0.0;
}

double TrainingSession::activeSeconds() const {
    if (state_ != SessionState::Running) {
        return accumulatedSeconds_;
    }
    const std::chrono::duration<double> elapsed = Clock::now() - runStarted_;
    return accumulatedSeconds_ + elapsed.count();
}

double TrainingSession::completionRate() const noexcept {
    if (targetCount_ <= 0) {
        return 0.0;
    }
    return std::clamp(
        static_cast<double>(validCount_) / static_cast<double>(targetCount_),
        0.0, 1.0);
}

SessionSummary TrainingSession::summary() const {
    const double duration = activeSeconds();
    return SessionSummary{
        exercise_,
        targetCount_,
        validCount_,
        invalidCount_,
        duration,
        completionRate(),
        validCount_ > 0 ? duration / static_cast<double>(validCount_) : 0.0
    };
}

} // namespace sport
