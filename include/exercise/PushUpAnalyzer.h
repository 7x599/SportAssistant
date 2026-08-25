#pragma once

#include "common/Config.h"
#include "exercise/ExerciseAnalyzer.h"
#include "exercise/PoseSmoother.h"

#include <optional>

namespace sport {

class PushUpAnalyzer final : public ExerciseAnalyzer {
public:
    explicit PushUpAnalyzer(AnalyzerConfig config = {});

    [[nodiscard]] ExerciseResult update(const Pose& pose) override;
    void reset() override;

private:
    enum class Phase { Up, Descending, Down, Ascending };

    struct SideAngles {
        double elbow{0.0};
        double bodyLine{0.0};
    };

    [[nodiscard]] std::optional<SideAngles> selectSide(const Pose& pose) const;
    [[nodiscard]] static const char* phaseText(Phase phase) noexcept;
    [[nodiscard]] ExerciseResult makeResult(
        bool hasPose,
        double elbowAngle,
        double bodyAngle,
        bool eventOccurred = false,
        bool eventValid = false,
        std::string message = {}) const;
    void changePhase(Phase phase) noexcept;

    AnalyzerConfig config_;
    PoseSmoother smoother_;
    Phase phase_{Phase::Up};
    Phase pendingPhase_{Phase::Up};
    int pendingFrames_{0};
    int lostPoseFrames_{0};
    int validCount_{0};
    int invalidCount_{0};
    double previousAngle_{180.0};
    double repStartTime_{0.0};
    double minimumAngle_{180.0};
    double minimumBodyAngle_{180.0};
    bool reachedDown_{false};
    std::string message_{"撑直后开始"};
};

} // namespace sport
