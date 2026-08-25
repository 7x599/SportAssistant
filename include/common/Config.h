#pragma once

namespace sport {

struct AnalyzerConfig {
    double confidenceThreshold{0.35};
    double smoothingAlpha{0.42};
    int stableFrames{2};
    int lostPoseResetFrames{18};
    double directionEpsilonDegrees{1.2};
    double minimumRepSeconds{0.55};
    double maximumRepSeconds{8.0};

    struct Squat {
        double standingAngle{155.0};
        double bottomAngle{100.0};
        double descentStartAngle{145.0};
    } squat;

    struct PushUp {
        double upAngle{155.0};
        double downAngle{90.0};
        double descentStartAngle{145.0};
        double minimumBodyLineAngle{155.0};
    } pushUp;
};

} // namespace sport
