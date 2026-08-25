#include "training/TrainingRecord.h"

#include "common/ExerciseType.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace sport {

TrainingRecord::TrainingRecord(std::filesystem::path csvPath)
    : csvPath_(std::move(csvPath)) {}

bool TrainingRecord::append(const SessionSummary& summary, std::string* error) const {
    try {
        if (csvPath_.has_parent_path()) {
            std::filesystem::create_directories(csvPath_.parent_path());
        }
        const bool needsHeader = !std::filesystem::exists(csvPath_) ||
                                 std::filesystem::file_size(csvPath_) == 0;
        std::ofstream stream(csvPath_, std::ios::app);
        if (!stream) {
            if (error) *error = "无法打开训练记录文件";
            return false;
        }
        if (needsHeader) {
            stream << "timestamp,exercise,target,valid,invalid,duration_seconds,completion_rate,average_rep_seconds\n";
        }

        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        stream << std::put_time(&local, "%Y-%m-%d %H:%M:%S") << ','
               << exerciseKey(summary.exercise) << ','
               << summary.targetCount << ','
               << summary.validCount << ','
               << summary.invalidCount << ','
               << std::fixed << std::setprecision(2)
               << summary.activeSeconds << ','
               << summary.completionRate << ','
               << summary.averageRepSeconds << '\n';
        return static_cast<bool>(stream);
    } catch (const std::exception& exception) {
        if (error) *error = exception.what();
        return false;
    }
}

} // namespace sport
