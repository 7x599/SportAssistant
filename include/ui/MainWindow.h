#pragma once

#include "common/ExerciseType.h"
#include "exercise/ExerciseAnalyzer.h"
#include "training/TrainingRecord.h"
#include "training/TrainingSession.h"
#include "vision/VideoPipeline.h"

#include <QMainWindow>
#include <QString>

#include <filesystem>
#include <memory>

class QComboBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTimer;

namespace sport {

struct LaunchOptions {
    InputMode inputMode{InputMode::Demo};
    int cameraIndex{0};
    QString videoPath;
    QString modelPath{"models/yolo11n-pose.onnx"};
};

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(LaunchOptions options, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void startTraining();
    void togglePause();
    void finishTraining();
    void returnHome();
    void repeatTraining();
    void selectVideoFile();
    void selectCamera();
    void selectDemo();
    void processFrame();

private:
    QWidget* buildHomePage();
    QWidget* buildTrainingPage();
    QWidget* buildResultPage();
    QWidget* buildTopBar(const QString& title, const QString& subtitle = {});
    QWidget* makeMetricRow(const QString& label, QLabel*& valueLabel, const QString& objectName);
    void configureInput();
    void setExercise(ExerciseType exercise);
    void resetAnalyzer();
    void updateTrainingUi(const ExerciseResult& result, double fps);
    void updateStatus(const ExerciseResult& result);
    void showFrame(const cv::Mat& frame);
    void applyStyle();
    [[nodiscard]] QString formatTime(double seconds) const;
    [[nodiscard]] std::filesystem::path recordPath() const;

    LaunchOptions options_;
    ExerciseType exercise_{ExerciseType::Squat};
    std::unique_ptr<ExerciseAnalyzer> analyzer_;
    VideoPipeline pipeline_;
    TrainingSession session_;
    TrainingRecord record_;

    QStackedWidget* pages_{nullptr};
    QComboBox* exerciseCombo_{nullptr};
    QSpinBox* targetSpin_{nullptr};
    QLabel* inputSummary_{nullptr};
    QLabel* videoLabel_{nullptr};
    QLabel* liveLabel_{nullptr};
    QLabel* exerciseLabel_{nullptr};
    QLabel* countLabel_{nullptr};
    QLabel* targetLabel_{nullptr};
    QLabel* progressLabel_{nullptr};
    QProgressBar* progressBar_{nullptr};
    QLabel* timeLabel_{nullptr};
    QLabel* angleLabel_{nullptr};
    QLabel* bodyAngleLabel_{nullptr};
    QLabel* invalidCountLabel_{nullptr};
    QLabel* phaseLabel_{nullptr};
    QLabel* statusLabel_{nullptr};
    QPushButton* pauseButton_{nullptr};
    QLabel* resultExercise_{nullptr};
    QLabel* resultCount_{nullptr};
    QLabel* resultValid_{nullptr};
    QLabel* resultInvalid_{nullptr};
    QLabel* resultDuration_{nullptr};
    QLabel* resultProgress_{nullptr};
    QLabel* resultAverage_{nullptr};
    QTimer* frameTimer_{nullptr};
};

} // namespace sport
