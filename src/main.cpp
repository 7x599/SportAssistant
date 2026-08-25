#include "ui/MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>

int main(int argc, char* argv[]) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName("SportAssistant");
    QCoreApplication::setOrganizationName("StudentTeam");
    QCommandLineParser parser;
    parser.setApplicationDescription("智能运动计数助手");
    parser.addHelpOption();
    parser.addOption({"demo", "使用内置演示姿态（默认）"});
    parser.addOption({"camera", "使用摄像头编号", "index"});
    parser.addOption({"video", "使用本地视频", "path"});
    parser.addOption({"model", "YOLO Pose ONNX 模型路径", "path", "models/yolo11n-pose.onnx"});
    parser.process(application);

    sport::LaunchOptions options;
    options.modelPath = parser.value("model");
    if (parser.isSet("video")) {
        options.inputMode = sport::InputMode::File;
        options.videoPath = parser.value("video");
    } else if (parser.isSet("camera")) {
        options.inputMode = sport::InputMode::Camera;
        options.cameraIndex = parser.value("camera").toInt();
    }

    sport::MainWindow window(options);
    window.show();
    return application.exec();
}
