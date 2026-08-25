#pragma once

#include "common/ExerciseResult.h"
#include "common/Pose.h"

namespace sport {

class ExerciseAnalyzer {
public:
    virtual ~ExerciseAnalyzer() = default;
    [[nodiscard]] virtual ExerciseResult update(const Pose& pose) = 0;
    virtual void reset() = 0;
};

} // namespace sport
