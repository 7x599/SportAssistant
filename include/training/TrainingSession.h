#pragma once

#include "common/ExerciseResult.h"
#include "common/ExerciseType.h"

#include <chrono>

namespace sport {

enum class SessionState { Idle, Running, Paused, Finished };

struct SessionSummary {
    ExerciseType exercise{ExerciseType::Squat};
    int targetCount{0};
    int validCount{0};
    int invalidCount{0};
    double activeSeconds{0.0};
    double completionRate{0.0};
    double averageRepSeconds{0.0};
};

class TrainingSession {
public:
    void start(ExerciseType exercise, int targetCount);
    void pause();
    void resume();
    [[nodiscard]] SessionSummary finish();
    void update(const ExerciseResult& result);
    void reset() noexcept;

    [[nodiscard]] SessionState state() const noexcept { return state_; }
    [[nodiscard]] bool isRunning() const noexcept { return state_ == SessionState::Running; }
    [[nodiscard]] int targetCount() const noexcept { return targetCount_; }
    [[nodiscard]] int validCount() const noexcept { return validCount_; }
    [[nodiscard]] int invalidCount() const noexcept { return invalidCount_; }
    [[nodiscard]] double activeSeconds() const;
    [[nodiscard]] double completionRate() const noexcept;
    [[nodiscard]] SessionSummary summary() const;

private:
    using Clock = std::chrono::steady_clock;

    ExerciseType exercise_{ExerciseType::Squat};
    SessionState state_{SessionState::Idle};
    int targetCount_{30};
    int validCount_{0};
    int invalidCount_{0};
    Clock::time_point runStarted_{};
    double accumulatedSeconds_{0.0};
};

} // namespace sport
