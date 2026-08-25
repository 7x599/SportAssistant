#pragma once

#include "training/TrainingSession.h"

#include <filesystem>
#include <string>

namespace sport {

class TrainingRecord {
public:
    explicit TrainingRecord(std::filesystem::path csvPath);

    [[nodiscard]] bool append(const SessionSummary& summary, std::string* error = nullptr) const;
    [[nodiscard]] const std::filesystem::path& path() const noexcept { return csvPath_; }

private:
    std::filesystem::path csvPath_;
};

} // namespace sport
