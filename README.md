# SportAssistant · 智能运动计数助手

一个可本地运行的 C++/Qt 桌面软件：摄像头或视频输入经过 YOLO Pose ONNX 得到 17 个关键点，再由成员 B 的角度计算与状态机完成深蹲、俯卧撑的有效计数、防误触和动作质量提示。

## 已实现

- Windows Qt Widgets 三页流程：首页 → 实时训练 → 训练结果。
- 深蹲与俯卧撑完整状态机，不完整周期不计数。
- EMA 关键点平滑、置信度过滤、关键点丢失保护。
- 深蹲：深度不足、未完成周期、动作过快判断。
- 俯卧撑：下压不足、未完全撑起、身体弯曲、动作过快判断。
- 摄像头、本地视频、内置确定性演示三种输入。
- OpenCV DNN 读取 Ultralytics YOLO11 Pose ONNX，CPU 可运行。
- 目标次数、暂停/继续、计时、完成率、有效/无效次数、平均周期。
- 训练结果自动保存为 CSV。
- 不依赖模型和相机的核心单元测试。

## 目录与成员边界

```text
include/common/       四人共用的数据结构（修改前先沟通）
include/vision/       A：PoseDetector、VideoPipeline
include/exercise/     B：角度、平滑、状态机
include/ui/           C：Qt 页面与交互
include/training/     D：会话、统计、记录
src/...               对应实现
resources/dark.qss    UI 设计系统
tests/core_tests.cpp  B 的确定性算法测试
scripts/              Windows 构建、模型导出、打包
docs/                 接口、分工部署、测试方案
```

稳定数据流：

```text
Camera / MP4 / Demo
        ↓
VideoPipeline + PoseDetector      (A)
        ↓ Pose
SquatAnalyzer / PushUpAnalyzer    (B)
        ↓ ExerciseResult
TrainingSession                   (D)
        ↓
MainWindow                        (C)
```

## 你（成员 B）的 Windows 本地部署

推荐在 Windows 10/11 的“Developer PowerShell for VS 2022”执行。

### 1. 安装工具

首次使用可由脚本安装 Git、CMake 和 Visual Studio C++ Build Tools：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\setup_windows.ps1 -InstallPrerequisites
```

安装结束后关闭窗口，重新打开“Developer PowerShell for VS 2022”。

### 2. 自动下载 Qt/OpenCV、编译、测试并启动演示

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\setup_windows.ps1
```

脚本会在项目内建立 `.tools/vcpkg`，通过 `vcpkg.json` 安装 Qt 6 和 OpenCV 4，然后编译并运行内置演示。第一次安装依赖时间较长。

也可以分步执行：

```powershell
.\scripts\build_windows.ps1
.\build\windows-release\Release\SportAssistant.exe --demo
```

演示模式不需要模型、摄像头或成员 A 的代码，能独立检查你负责的深蹲/俯卧撑状态机、UI、训练统计和 CSV。

### 3. 接入真实姿态模型

电脑已安装 Python 3.10+ 后执行：

```powershell
.\scripts\export_model.ps1
.\build\windows-release\Release\SportAssistant.exe --camera 0
```

也可使用视频：

```powershell
.\build\windows-release\Release\SportAssistant.exe --video .\test_videos\squat.mp4
```

其他模型路径：

```powershell
.\build\windows-release\Release\SportAssistant.exe --model D:\models\pose.onnx --camera 0
```

## 运行验证

完整构建脚本会自动执行 CTest。只验证成员 B 的算法时，可关闭 GUI 依赖：

```powershell
cmake -S . -B build\core-only -DSPORTASSISTANT_BUILD_GUI=OFF -DSPORTASSISTANT_BUILD_TESTS=ON
cmake --build build\core-only --config Debug
ctest --test-dir build\core-only -C Debug --output-on-failure
```

预期输出：`All SportAssistant core tests passed.`

## 生成可提交程序

```powershell
.\scripts\package_windows.ps1
```

结果在 `dist\SportAssistant\`。脚本会收集 Qt、OpenCV 和 MSVC 运行库；模型存在时也会复制。提交前请在没有开发环境的另一台 Windows 电脑上运行一次 `SportAssistant.exe --demo`。

CSV 默认位于 `%LOCALAPPDATA%\StudentTeam\SportAssistant\data\training_records.csv`，避免软件放在只读目录时保存失败。

## GitHub 协作

```text
main                 稳定提交版
develop              每 1–2 天集成
feature/pose         A
feature/exercise     B（你）
feature/ui           C
feature/training     D
```

首次初始化本地分支可运行：

```powershell
.\scripts\init_git_branches.ps1
```

每位成员自行 commit/push；你负责检查共享接口与 PR，先合并到 `develop`，通过核心测试和演示验收后再合并到 `main`。详细职责见 [docs/member_deployment.md](docs/member_deployment.md)。

## 阈值调参位置

所有初始阈值集中在 `include/common/Config.h`：

- 深蹲站立角 `155°`，底部角 `100°`；
- 俯卧撑撑起角 `155°`，底部角 `90°`；
- 俯卧撑身体线最低 `155°`；
- 最短动作周期 `0.55 s`。

这些是初值，不是最终实验结论。D 收集视频测试结果后，由 B 修改并记录一版冻结参数。

## 常见问题

- 摄像头打不开：先关闭系统相机、会议软件；尝试 `--camera 1`；程序会自动回退演示模式。
- 有画面无关键点：检查 `models/yolo11n-pose.onnx` 是否存在；确保人物全身入镜且光线充足。
- DJI 设备：必须在 Windows 中表现为可选摄像头（UVC）才能直接 `--camera`；否则先录制 MP4 或使用采集卡。
- Qt/OpenCV 找不到：确认在项目根目录运行脚本，且 `VCPKG_ROOT` 没有被设置成另一个残缺目录。
- 计数不稳定：先用 `--video` 固定输入复现，再由 B 调 `Config.h`，不要在 UI 中写角度判断。
