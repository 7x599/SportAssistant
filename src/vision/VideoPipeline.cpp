#include "vision/VideoPipeline.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <exception>

namespace sport {

    namespace {
        // Lower these limits for less image-processing work; raise for more detail.
        constexpr int kProcessingWidth = 640;
        constexpr int kProcessingHeight = 360;
        constexpr double kRequestedCameraFps = 30.0;

        // Keep the whole frame and its aspect ratio. Detect and draw on the same
        // resized image so pixel-based pose coordinates still match the image.
        void limitFrameSize(cv::Mat& image) {
            if (image.cols <= kProcessingWidth && image.rows <= kProcessingHeight) return;
            const double scale = (std::min)(
                static_cast<double>(kProcessingWidth) / image.cols,
                static_cast<double>(kProcessingHeight) / image.rows);
            const int width = (std::max)(1, static_cast<int>(std::lround(image.cols * scale)));
            const int height = (std::max)(1, static_cast<int>(std::lround(image.rows * scale)));
            cv::resize(image, image, cv::Size(width, height), 0.0, 0.0, cv::INTER_AREA);
        }
    } // namespace

    bool VideoPipeline::loadModel(const std::filesystem::path& modelPath) {
        const bool loaded = detector_.load(modelPath);
        if (!loaded) lastError_ = detector_.lastError();
        return loaded;
    }

    bool VideoPipeline::openCamera(int index) {
        close();
#ifdef _WIN32
        const bool opened = capture_.open(index, cv::CAP_DSHOW);
#else
        const bool opened = capture_.open(index);
#endif
        if (!opened) {
            lastError_ = "无法打开摄像头，已切换到演示模式";
            openDemo(exercise_);
            return false;
        }
        // Camera drivers may choose a different supported resolution/frame rate.
        capture_.set(cv::CAP_PROP_FRAME_WIDTH, kProcessingWidth);
        capture_.set(cv::CAP_PROP_FRAME_HEIGHT, kProcessingHeight);
        capture_.set(cv::CAP_PROP_FPS, kRequestedCameraFps);
        capture_.set(cv::CAP_PROP_BUFFERSIZE, 1);
        mode_ = InputMode::Camera;
        started_ = Clock::now();
        previousFrame_ = started_;
        lastError_.clear();
        return true;
    }

    bool VideoPipeline::openFile(const std::filesystem::path& path) {
        close();
        if (!capture_.open(path.string())) {
            lastError_ = "无法打开视频文件";
            openDemo(exercise_);
            return false;
        }
        mode_ = InputMode::File;
        started_ = Clock::now();
        previousFrame_ = started_;
        lastError_.clear();
        return true;
    }

    void VideoPipeline::openDemo(ExerciseType exercise) {
        capture_.release();
        exercise_ = exercise;
        demo_.reset(exercise);
        mode_ = InputMode::Demo;
        started_ = Clock::now();
        previousFrame_ = started_;
    }

    void VideoPipeline::setExercise(ExerciseType exercise) {
        exercise_ = exercise;
        if (mode_ == InputMode::Demo) demo_.reset(exercise);
    }

    ProcessedFrame VideoPipeline::read() {
        ProcessedFrame result;
        const auto now = Clock::now();
        const std::chrono::duration<double> fromPrevious = now - previousFrame_;
        previousFrame_ = now;
        result.fps = fromPrevious.count() > 1.0e-6 ? 1.0 / fromPrevious.count() : 0.0;

        if (mode_ == InputMode::Demo) {
            result.pose = demo_.next(result.image);
            result.hasFrame = true;
            result.hasPose = result.pose.score > 0.0F;
            return result;
        }

        if (!capture_.read(result.image) || result.image.empty()) {
            if (mode_ == InputMode::File) {
                capture_.set(cv::CAP_PROP_POS_FRAMES, 0.0);
                capture_.read(result.image);
            }
            if (result.image.empty()) {
                lastError_ = "视频流已中断";
                return result;
            }
        }

        const std::chrono::duration<double> elapsed = now - started_;
        result.hasFrame = true;
        try {
            // Also bounds file frames and camera frames when the driver ignores
            // the requested capture size. Keep inference on every frame.
            limitFrameSize(result.image);
            result.pose = detector_.detect(result.image, elapsed.count());
            result.hasPose = result.pose.score > 0.0F;
            if (result.hasPose) detector_.drawPose(result.image, result.pose);
        }
        catch (const std::exception& exception) {
            lastError_ = std::string("姿态推理失败: ") + exception.what();
            result.hasPose = false;
        }
        return result;
    }

    void VideoPipeline::close() {
        capture_.release();
    }

} // namespace sport
