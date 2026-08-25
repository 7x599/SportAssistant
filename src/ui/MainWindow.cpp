/*
THESIS: Turn opaque AI counting into a legible training instrument; refuse the generic dashboard grid.
OWN-WORLD: Graphite and deep-petrol planes, cyan/mint state ink, one amber warning channel, crisp one-pixel rules and 6px corners.
STORY: Select an exercise, watch the pose and phase evidence, complete valid repetitions, then leave with a local record.
FIRST VIEWPORT: A 64% live stage anchors the left; the right performance rail gives count, progress, time, angle, phase and form; pause and finish remain in a bottom command dock.
FORM: Native Windows sports-lab console, first-ranked grounded direction; online seed unavailable in this environment.
FINISH: unreviewed and undocumented is unfinished; this build ends with the finish review, the verdict, and DESIGN.md
*/

#include "ui/MainWindow.h"

#include "exercise/PushUpAnalyzer.h"
#include "exercise/SquatAnalyzer.h"

#include <opencv2/imgproc.hpp>

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QPixmap>
#include <QShortcut>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

namespace sport {

namespace {

QFrame* divider(QFrame::Shape shape = QFrame::HLine) {
    auto* line = new QFrame;
    line->setObjectName("Divider");
    line->setFrameShape(shape);
    line->setFrameShadow(QFrame::Plain);
    return line;
}

QLabel* textLabel(const QString& text, const char* objectName = "BodyText") {
    auto* label = new QLabel(text);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    return label;
}

} // namespace

MainWindow::MainWindow(LaunchOptions options, QWidget* parent)
    : QMainWindow(parent),
      options_(std::move(options)),
      record_(recordPath()) {
    setWindowTitle("智能运动计数助手 · SportAssistant");
    setMinimumSize(1060, 680);
    resize(1440, 900);
    pages_ = new QStackedWidget;
    pages_->setObjectName("AppPages");
    pages_->addWidget(buildHomePage());
    pages_->addWidget(buildTrainingPage());
    pages_->addWidget(buildResultPage());
    setCentralWidget(pages_);

    frameTimer_ = new QTimer(this);
    frameTimer_->setTimerType(Qt::PreciseTimer);
    frameTimer_->setInterval(33);
    connect(frameTimer_, &QTimer::timeout, this, &MainWindow::processFrame);

    auto* pauseShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    auto* finishShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(pauseShortcut, &QShortcut::activated, this, &MainWindow::togglePause);
    connect(finishShortcut, &QShortcut::activated, this, &MainWindow::finishTraining);

    resetAnalyzer();
    configureInput();
    applyStyle();
}

MainWindow::~MainWindow() = default;

QWidget* MainWindow::buildTopBar(const QString& title, const QString& subtitle) {
    auto* bar = new QWidget;
    bar->setObjectName("TopBar");
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(26, 18, 26, 18);
    layout->setSpacing(16);

    auto* mark = new QLabel("SA");
    mark->setObjectName("BrandMark");
    mark->setAlignment(Qt::AlignCenter);
    mark->setFixedSize(40, 40);
    layout->addWidget(mark);

    auto* titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(1);
    auto* heading = new QLabel(title);
    heading->setObjectName("TopTitle");
    titleBlock->addWidget(heading);
    if (!subtitle.isEmpty()) {
        auto* detail = new QLabel(subtitle);
        detail->setObjectName("TopSubtitle");
        titleBlock->addWidget(detail);
    }
    layout->addLayout(titleBlock);
    layout->addStretch();

    auto* buildTag = new QLabel("LOCAL · C++ / QT");
    buildTag->setObjectName("BuildTag");
    layout->addWidget(buildTag);
    return bar;
}

QWidget* MainWindow::buildHomePage() {
    auto* page = new QWidget;
    page->setObjectName("HomePage");
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(buildTopBar("智能运动计数助手", "SPORTASSISTANT · 可解释的实时动作计数"));
    root->addWidget(divider());

    auto* content = new QHBoxLayout;
    content->setContentsMargins(54, 46, 54, 46);
    content->setSpacing(58);

    auto* statement = new QVBoxLayout;
    statement->setSpacing(18);
    auto* title = new QLabel("把每一次计数，\n变成看得见的证据。");
    title->setObjectName("HomeHeadline");
    title->setWordWrap(true);
    statement->addWidget(title);
    statement->addWidget(textLabel(
        "摄像头画面、人体关键点、关节角度与动作相位在同一个窗口中同步呈现。"
        "完整状态机只接受一次完整动作周期。", "HomeLead"));
    auto* proof = new QLabel("KEYPOINTS  →  ANGLE  →  PHASE  →  VALID REP");
    proof->setObjectName("PipelineText");
    statement->addWidget(proof);
    statement->addStretch();
    auto* fallback = textLabel(
        "答辩保障：支持摄像头、本地 MP4 与内置演示三种输入。模型或设备不可用时，演示模式仍可验证成员 B 的全部算法与 UI 链路。",
        "MutedText");
    fallback->setMaximumWidth(560);
    statement->addWidget(fallback);
    content->addLayout(statement, 6);

    auto* setup = new QWidget;
    setup->setObjectName("SetupPanel");
    setup->setMinimumWidth(390);
    setup->setMaximumWidth(480);
    auto* form = new QVBoxLayout(setup);
    form->setContentsMargins(30, 30, 30, 30);
    form->setSpacing(16);
    auto* setupTitle = new QLabel("开始一次训练");
    setupTitle->setObjectName("SectionTitle");
    form->addWidget(setupTitle);

    form->addWidget(textLabel("动作类型", "FieldLabel"));
    exerciseCombo_ = new QComboBox;
    exerciseCombo_->addItem("深蹲 · 膝关节状态机", static_cast<int>(ExerciseType::Squat));
    exerciseCombo_->addItem("俯卧撑 · 肘关节状态机", static_cast<int>(ExerciseType::PushUp));
    form->addWidget(exerciseCombo_);

    form->addWidget(textLabel("目标次数", "FieldLabel"));
    targetSpin_ = new QSpinBox;
    targetSpin_->setRange(1, 999);
    targetSpin_->setValue(30);
    targetSpin_->setSuffix(" 次");
    form->addWidget(targetSpin_);

    form->addWidget(textLabel("输入源", "FieldLabel"));
    inputSummary_ = textLabel("内置演示", "InputSummary");
    form->addWidget(inputSummary_);
    auto* sources = new QHBoxLayout;
    sources->setSpacing(8);
    auto* demoButton = new QPushButton("演示");
    auto* cameraButton = new QPushButton("摄像头");
    auto* fileButton = new QPushButton("视频文件");
    demoButton->setObjectName("QuietButton");
    cameraButton->setObjectName("QuietButton");
    fileButton->setObjectName("QuietButton");
    sources->addWidget(demoButton);
    sources->addWidget(cameraButton);
    sources->addWidget(fileButton);
    form->addLayout(sources);
    connect(demoButton, &QPushButton::clicked, this, &MainWindow::selectDemo);
    connect(cameraButton, &QPushButton::clicked, this, &MainWindow::selectCamera);
    connect(fileButton, &QPushButton::clicked, this, &MainWindow::selectVideoFile);

    form->addSpacing(8);
    auto* startButton = new QPushButton("开始训练");
    startButton->setObjectName("PrimaryButton");
    startButton->setMinimumHeight(58);
    startButton->setDefault(true);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startTraining);
    form->addWidget(startButton);

    form->addWidget(textLabel("训练记录将自动保存为本地 CSV。", "MutedText"));
    form->addStretch();
    content->addWidget(setup, 4);
    root->addLayout(content, 1);
    return page;
}

QWidget* MainWindow::makeMetricRow(
    const QString& label, QLabel*& valueLabel, const QString& objectName) {
    auto* row = new QWidget;
    row->setObjectName("MetricRow");
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(18, 14, 18, 14);
    auto* name = new QLabel(label);
    name->setObjectName("MetricName");
    valueLabel = new QLabel("—");
    valueLabel->setObjectName(objectName);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(name);
    layout->addStretch();
    layout->addWidget(valueLabel);
    return row;
}

QWidget* MainWindow::buildTrainingPage() {
    auto* page = new QWidget;
    page->setObjectName("TrainingPage");
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(buildTopBar("智能运动计数助手", "实时训练 · 空格暂停 / ESC 结束"));
    root->addWidget(divider());

    auto* work = new QHBoxLayout;
    work->setContentsMargins(16, 16, 16, 0);
    work->setSpacing(14);

    auto* stage = new QWidget;
    stage->setObjectName("VideoStage");
    auto* stageLayout = new QVBoxLayout(stage);
    stageLayout->setContentsMargins(0, 0, 0, 0);
    stageLayout->setSpacing(0);
    auto* liveStrip = new QWidget;
    liveStrip->setObjectName("LiveStrip");
    auto* liveLayout = new QHBoxLayout(liveStrip);
    liveLayout->setContentsMargins(18, 11, 18, 11);
    liveLabel_ = new QLabel("LIVE  ·  DEMO");
    liveLabel_->setObjectName("LiveText");
    liveLayout->addWidget(liveLabel_);
    liveLayout->addStretch();
    auto* evidence = new QLabel("POSE / ANGLE / PHASE");
    evidence->setObjectName("EvidenceText");
    liveLayout->addWidget(evidence);
    stageLayout->addWidget(liveStrip);
    videoLabel_ = new QLabel("等待视频输入");
    videoLabel_->setObjectName("VideoLabel");
    videoLabel_->setAlignment(Qt::AlignCenter);
    videoLabel_->setAccessibleName("实时视频与人体姿态骨架");
    videoLabel_->setMinimumSize(620, 430);
    videoLabel_->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    stageLayout->addWidget(videoLabel_, 1);
    work->addWidget(stage, 64);

    auto* rail = new QWidget;
    rail->setObjectName("PerformanceRail");
    rail->setMinimumWidth(350);
    rail->setMaximumWidth(480);
    auto* metrics = new QVBoxLayout(rail);
    metrics->setContentsMargins(22, 20, 22, 20);
    metrics->setSpacing(10);
    exerciseLabel_ = new QLabel("深蹲");
    exerciseLabel_->setObjectName("ExerciseTitle");
    metrics->addWidget(exerciseLabel_);

    auto* countLine = new QHBoxLayout;
    countLine->setSpacing(12);
    countLabel_ = new QLabel("0");
    countLabel_->setObjectName("CountNumber");
    targetLabel_ = new QLabel("/ 30");
    targetLabel_->setObjectName("CountTarget");
    targetLabel_->setAlignment(Qt::AlignBottom);
    countLine->addWidget(countLabel_);
    countLine->addWidget(targetLabel_);
    countLine->addStretch();
    metrics->addLayout(countLine);

    auto* progressLine = new QHBoxLayout;
    auto* progressName = new QLabel("完成率");
    progressName->setObjectName("MetricName");
    progressLabel_ = new QLabel("0%");
    progressLabel_->setObjectName("ProgressNumber");
    progressLine->addWidget(progressName);
    progressLine->addStretch();
    progressLine->addWidget(progressLabel_);
    metrics->addLayout(progressLine);
    progressBar_ = new QProgressBar;
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setAccessibleName("训练完成率");
    metrics->addWidget(progressBar_);

    metrics->addWidget(divider());
    metrics->addWidget(makeMetricRow("训练时间", timeLabel_, "MetricValue"));
    metrics->addWidget(makeMetricRow("主关节角度", angleLabel_, "MetricAccent"));
    metrics->addWidget(makeMetricRow("身体线角度", bodyAngleLabel_, "MetricValue"));
    metrics->addWidget(makeMetricRow("无效动作", invalidCountLabel_, "MetricWarning"));
    metrics->addWidget(makeMetricRow("当前阶段", phaseLabel_, "MetricAccent"));

    statusLabel_ = new QLabel("等待姿态");
    statusLabel_->setObjectName("StatusPanel");
    statusLabel_->setAccessibleName("动作质量反馈");
    statusLabel_->setProperty("state", "neutral");
    statusLabel_->setWordWrap(true);
    statusLabel_->setMinimumHeight(72);
    metrics->addWidget(statusLabel_);
    metrics->addStretch();
    work->addWidget(rail, 36);
    root->addLayout(work, 1);

    auto* dock = new QWidget;
    dock->setObjectName("CommandDock");
    auto* commands = new QHBoxLayout(dock);
    commands->setContentsMargins(28, 18, 28, 20);
    commands->setSpacing(18);
    pauseButton_ = new QPushButton("暂停");
    pauseButton_->setObjectName("SecondaryButton");
    pauseButton_->setMinimumHeight(54);
    auto* finishButton = new QPushButton("结束训练");
    finishButton->setObjectName("DangerButton");
    finishButton->setMinimumHeight(54);
    commands->addWidget(pauseButton_);
    commands->addWidget(finishButton);
    connect(pauseButton_, &QPushButton::clicked, this, &MainWindow::togglePause);
    connect(finishButton, &QPushButton::clicked, this, &MainWindow::finishTraining);
    root->addWidget(dock);
    return page;
}

QWidget* MainWindow::buildResultPage() {
    auto* page = new QWidget;
    page->setObjectName("ResultPage");
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(buildTopBar("训练完成", "结果已保存到本地记录"));
    root->addWidget(divider());

    auto* body = new QHBoxLayout;
    body->setContentsMargins(70, 54, 70, 54);
    body->setSpacing(72);
    auto* hero = new QVBoxLayout;
    resultExercise_ = new QLabel("深蹲");
    resultExercise_->setObjectName("ResultExercise");
    resultCount_ = new QLabel("0 / 30");
    resultCount_->setObjectName("ResultCount");
    auto* resultCaption = textLabel("有效动作 / 目标", "MutedText");
    hero->addWidget(resultExercise_);
    hero->addWidget(resultCount_);
    hero->addWidget(resultCaption);
    hero->addStretch();
    auto* repeat = new QPushButton("再次训练");
    repeat->setObjectName("PrimaryButton");
    repeat->setMinimumHeight(56);
    auto* home = new QPushButton("返回首页");
    home->setObjectName("SecondaryButton");
    home->setMinimumHeight(56);
    connect(repeat, &QPushButton::clicked, this, &MainWindow::repeatTraining);
    connect(home, &QPushButton::clicked, this, &MainWindow::returnHome);
    hero->addWidget(repeat);
    hero->addWidget(home);
    body->addLayout(hero, 5);

    auto* report = new QWidget;
    report->setObjectName("ResultReport");
    auto* rows = new QVBoxLayout(report);
    rows->setContentsMargins(28, 24, 28, 24);
    rows->setSpacing(8);
    auto* reportTitle = new QLabel("本次训练摘要");
    reportTitle->setObjectName("SectionTitle");
    rows->addWidget(reportTitle);
    rows->addWidget(makeMetricRow("有效动作", resultValid_, "MetricAccent"));
    rows->addWidget(makeMetricRow("无效动作", resultInvalid_, "MetricWarning"));
    rows->addWidget(makeMetricRow("训练时间", resultDuration_, "MetricValue"));
    rows->addWidget(makeMetricRow("完成率", resultProgress_, "MetricAccent"));
    rows->addWidget(makeMetricRow("平均周期", resultAverage_, "MetricValue"));
    rows->addStretch();
    rows->addWidget(textLabel(
        "无效动作不会增加完成率。历史记录保存在 Windows 应用数据目录的 data/training_records.csv。",
        "MutedText"));
    body->addWidget(report, 5);
    root->addLayout(body, 1);
    return page;
}

void MainWindow::configureInput() {
    std::filesystem::path modelPath(options_.modelPath.toStdString());
    if (modelPath.is_relative() && !std::filesystem::exists(modelPath)) {
        modelPath = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()) /
                    modelPath;
    }
    pipeline_.loadModel(modelPath);
    switch (options_.inputMode) {
    case InputMode::Camera:
        pipeline_.openCamera(options_.cameraIndex);
        break;
    case InputMode::File:
        pipeline_.openFile(options_.videoPath.toStdString());
        break;
    case InputMode::Demo:
        pipeline_.openDemo(exercise_);
        break;
    }
    if (!inputSummary_) return;
    switch (pipeline_.mode()) {
    case InputMode::Camera: inputSummary_->setText(QString("摄像头 %1").arg(options_.cameraIndex)); break;
    case InputMode::File: inputSummary_->setText(QString("视频：%1").arg(options_.videoPath)); break;
    case InputMode::Demo: inputSummary_->setText("内置演示 · 不依赖模型或摄像头"); break;
    }
}

void MainWindow::setExercise(ExerciseType exercise) {
    exercise_ = exercise;
    pipeline_.setExercise(exercise);
    resetAnalyzer();
}

void MainWindow::resetAnalyzer() {
    if (exercise_ == ExerciseType::Squat) {
        analyzer_ = std::make_unique<SquatAnalyzer>();
    } else {
        analyzer_ = std::make_unique<PushUpAnalyzer>();
    }
}

void MainWindow::startTraining() {
    setExercise(static_cast<ExerciseType>(exerciseCombo_->currentData().toInt()));
    session_.start(exercise_, targetSpin_->value());
    exerciseLabel_->setText(QString::fromUtf8(exerciseName(exercise_).data()));
    targetLabel_->setText(QString("/ %1").arg(targetSpin_->value()));
    pauseButton_->setText("暂停");
    countLabel_->setText("0");
    invalidCountLabel_->setText("0");
    progressBar_->setValue(0);
    pages_->setCurrentIndex(1);
    frameTimer_->start();
}

void MainWindow::togglePause() {
    if (pages_->currentIndex() != 1) return;
    if (session_.state() == SessionState::Running) {
        session_.pause();
        pauseButton_->setText("继续");
        statusLabel_->setText("训练已暂停 · 再次按空格继续");
        statusLabel_->setProperty("state", "neutral");
    } else if (session_.state() == SessionState::Paused) {
        session_.resume();
        pauseButton_->setText("暂停");
    }
    statusLabel_->style()->unpolish(statusLabel_);
    statusLabel_->style()->polish(statusLabel_);
}

void MainWindow::finishTraining() {
    if (pages_->currentIndex() != 1 ||
        (session_.state() != SessionState::Running && session_.state() != SessionState::Paused)) {
        return;
    }
    frameTimer_->stop();
    const SessionSummary summary = session_.finish();
    std::string recordError;
    if (!record_.append(summary, &recordError)) {
        QMessageBox::warning(this, "记录保存失败", QString::fromStdString(recordError));
    }

    resultExercise_->setText(QString::fromUtf8(exerciseName(summary.exercise).data()));
    resultCount_->setText(QString("%1 / %2").arg(summary.validCount).arg(summary.targetCount));
    resultValid_->setText(QString::number(summary.validCount));
    resultInvalid_->setText(QString::number(summary.invalidCount));
    resultDuration_->setText(formatTime(summary.activeSeconds));
    resultProgress_->setText(QString("%1%").arg(std::lround(summary.completionRate * 100.0)));
    resultAverage_->setText(summary.validCount > 0
                                ? QString("%1 秒/次").arg(summary.averageRepSeconds, 0, 'f', 1)
                                : "—");
    pages_->setCurrentIndex(2);
}

void MainWindow::returnHome() {
    frameTimer_->stop();
    session_.reset();
    pages_->setCurrentIndex(0);
}

void MainWindow::repeatTraining() {
    pages_->setCurrentIndex(0);
    startTraining();
}

void MainWindow::selectVideoFile() {
    const QString path = QFileDialog::getOpenFileName(
        this, "选择训练视频", {}, "视频文件 (*.mp4 *.avi *.mov *.mkv)");
    if (path.isEmpty()) return;
    options_.inputMode = InputMode::File;
    options_.videoPath = path;
    configureInput();
}

void MainWindow::selectCamera() {
    options_.inputMode = InputMode::Camera;
    configureInput();
    if (pipeline_.mode() != InputMode::Camera) {
        QMessageBox::information(this, "摄像头不可用", "未能打开摄像头，已自动切换到演示模式。请检查 USB 连接和系统相机权限。 ");
    }
}

void MainWindow::selectDemo() {
    options_.inputMode = InputMode::Demo;
    configureInput();
}

void MainWindow::processFrame() {
    ProcessedFrame frame = pipeline_.read();
    if (!frame.hasFrame) {
        liveLabel_->setText("INPUT LOST");
        statusLabel_->setText("视频流中断 · 请返回首页切换输入源");
        statusLabel_->setProperty("state", "warning");
        statusLabel_->style()->unpolish(statusLabel_);
        statusLabel_->style()->polish(statusLabel_);
        return;
    }
    showFrame(frame.image);
    const QString source = pipeline_.mode() == InputMode::Demo ? "DEMO" :
                           pipeline_.mode() == InputMode::Camera ? "CAMERA" : "VIDEO";
    liveLabel_->setText(QString("LIVE  ·  %1  ·  FPS %2")
                            .arg(source)
                            .arg(std::clamp(frame.fps, 0.0, 99.0), 0, 'f', 0));

    if (!session_.isRunning()) {
        timeLabel_->setText(formatTime(session_.activeSeconds()));
        return;
    }
    const ExerciseResult result = analyzer_->update(frame.pose);
    updateTrainingUi(result, frame.fps);
}

void MainWindow::updateTrainingUi(const ExerciseResult& result, double) {
    session_.update(result);
    countLabel_->setText(QString::number(session_.validCount()));
    const int progress = static_cast<int>(std::lround(session_.completionRate() * 100.0));
    progressLabel_->setText(QString("%1%").arg(progress));
    progressBar_->setValue(progress);
    timeLabel_->setText(formatTime(session_.activeSeconds()));
    angleLabel_->setText(result.hasPose ? QString("%1°").arg(std::lround(result.mainAngle)) : "—");
    bodyAngleLabel_->setText(result.hasPose ? QString("%1°").arg(std::lround(result.bodyAngle)) : "—");
    invalidCountLabel_->setText(QString::number(session_.invalidCount()));
    phaseLabel_->setText(QString::fromStdString(result.phase));
    updateStatus(result);
}

void MainWindow::updateStatus(const ExerciseResult& result) {
    statusLabel_->setText(QString::fromStdString(result.message));
    const char* state = "neutral";
    if (!result.hasPose || (result.eventOccurred && !result.eventValid)) {
        state = "warning";
    } else if (result.eventValid || result.message == "动作标准" ||
               result.message.find("达标") != std::string::npos) {
        state = "ok";
    }
    statusLabel_->setProperty("state", state);
    statusLabel_->style()->unpolish(statusLabel_);
    statusLabel_->style()->polish(statusLabel_);
}

void MainWindow::showFrame(const cv::Mat& frame) {
    if (frame.empty()) return;
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    const QImage image(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    videoLabel_->setPixmap(QPixmap::fromImage(image.copy()).scaled(
        videoLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::applyStyle() {
    QFile stylesheet(":/styles/resources/dark.qss");
    if (stylesheet.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    }
}

QString MainWindow::formatTime(double seconds) const {
    const int total = std::max(0, static_cast<int>(std::lround(seconds)));
    return QString("%1:%2")
        .arg(total / 60, 2, 10, QLatin1Char('0'))
        .arg(total % 60, 2, 10, QLatin1Char('0'));
}

std::filesystem::path MainWindow::recordPath() const {
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return std::filesystem::path(root.toStdString()) / "data" / "training_records.csv";
}

} // namespace sport
