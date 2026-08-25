#pragma once

#include "common/ExerciseType.h"

#include <string>

namespace sport {

struct ExerciseResult {
    ExerciseType exercise{ExerciseType::Squat};
    int validCount{0};
    int invalidCount{0};
    double mainAngle{0.0};
    double bodyAngle{0.0};
    bool hasPose{false};
    bool eventOccurred{false};
    bool eventValid{false};
    std::string phase{"等待姿态"};
    std::string message{"请进入画面"};
};

} // namespace sport
