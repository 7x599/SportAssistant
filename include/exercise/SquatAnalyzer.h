#pragma once

#include "common/Config.h"
#include "exercise/ExerciseAnalyzer.h"
#include "exercise/PoseSmoother.h"

#include <optional>

namespace sport {

class SquatAnalyzer final : public ExerciseAnalyzer {
public:
    explicit SquatAnalyzer(AnalyzerConfig config = {});

    [[nodiscard]] ExerciseResult update(const Pose& pose) override;
    void reset() override;

private:
    enum class Phase { Standing, Descending, Bottom, Ascending };

    struct SideAngles {
        double knee{0.0};
        double torso{0.0};
    };

    [[nodiscard]] std::optional<SideAngles> selectSide(const Pose& pose) const;
    [[nodiscard]] static const char* phaseText(Phase phase) noexcept;
    [[nodiscard]] ExerciseResult makeResult(
        bool hasPose,
        double kneeAngle,
        double torsoAngle,
        bool eventOccurred = false,
        bool eventValid = false,
        std::string message = {}) const;
    void changePhase(Phase phase) noexcept;

    AnalyzerConfig config_;
    PoseSmoother smoother_;
    Phase phase_{Phase::Standing};
    Phase pendingPhase_{Phase::Standing};
    int pendingFrames_{0};
    int lostPoseFrames_{0};
    int validCount_{0};
    int invalidCount_{0};
    double previousAngle_{180.0};
    double repStartTime_{0.0};
    double minimumAngle_{180.0};
    bool reachedBottom_{false};
    std::string message_{"站直后开始"};
};

} // namespace sport
