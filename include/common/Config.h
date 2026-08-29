#pragma once

namespace sport {

struct AnalyzerConfig {
    double confidenceThreshold{0.35};//姿态检测置信度阈值
    double smoothingAlpha{0.42};//角度平滑系数
    int stableFrames{2};//稳定帧数阈值
    int lostPoseResetFrames{18}; //丢帧重置帧数
    double directionEpsilonDegrees{1.2};//角度方向容差（判断上升还是下降的阈值）
    double minimumRepSeconds{0.55};
    double maximumRepSeconds{8.0};//单次动作的最短 / 最长时长（过滤无效计数）

    struct Squat {
        double standingAngle{155.0};//站立姿态的膝盖参考角度
        double bottomAngle{100.0};//蹲到底的膝盖参考角度
        double descentStartAngle{145.0};//下蹲起始触发角度
    } squat;

    struct PushUp {
        double upAngle{155.0};
        double downAngle{90.0};
        double descentStartAngle{145.0};
        double minimumBodyLineAngle{155.0};//身体直线最小角度
    } pushUp;
};

} // namespace sport
