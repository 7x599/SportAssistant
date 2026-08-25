#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace sport {

enum class KeypointId : std::size_t {
    Nose = 0,
    LeftEye,
    RightEye,
    LeftEar,
    RightEar,
    LeftShoulder,
    RightShoulder,
    LeftElbow,
    RightElbow,
    LeftWrist,
    RightWrist,
    LeftHip,
    RightHip,
    LeftKnee,
    RightKnee,
    LeftAnkle,
    RightAnkle,
    Count
};

struct KeyPoint {
    float x{0.0F};
    float y{0.0F};
    float confidence{0.0F};

    [[nodiscard]] bool visible(float threshold = 0.35F) const noexcept {
        return confidence >= threshold;
    }
};

struct Pose {
    static constexpr std::size_t KeypointCount =
        static_cast<std::size_t>(KeypointId::Count);

    std::array<KeyPoint, KeypointCount> keypoints{};
    float score{0.0F};
    std::uint64_t frameIndex{0};
    double timestampSeconds{0.0};

    [[nodiscard]] const KeyPoint& at(KeypointId id) const noexcept {
        return keypoints[static_cast<std::size_t>(id)];
    }

    KeyPoint& at(KeypointId id) noexcept {
        return keypoints[static_cast<std::size_t>(id)];
    }
};

} // namespace sport
