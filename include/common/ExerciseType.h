#pragma once

#include <string_view>

namespace sport {

enum class ExerciseType {
    Squat,
    PushUp
};

[[nodiscard]] constexpr std::string_view exerciseName(ExerciseType type) noexcept {
    return type == ExerciseType::Squat ? "深蹲" : "俯卧撑";
}

[[nodiscard]] constexpr std::string_view exerciseKey(ExerciseType type) noexcept {
    return type == ExerciseType::Squat ? "squat" : "pushup";
}

} // namespace sport
