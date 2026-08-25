# 四人剩余部署与交付

所有成员先完成 README 的 Windows 工具安装并运行一次 `SportAssistant.exe --demo`。只有 A 需要模型才能完成主责；C、D 在等待 A 时均可用演示模式独立开发。

## A：视觉感知负责人

分支：`feature/pose`

1. 运行 `scripts/export_model.ps1`，确认生成 `models/yolo11n-pose.onnx`。
2. 运行 `SportAssistant.exe --camera 0`，完成摄像头 → 模型 → 17 点 → 骨架。
3. 用 `--video` 校验固定视频，避免调试时输入不断变化。
4. 若改用其他模型，只改 `PoseDetector` 内部解析，保持 `Pose.h` 不动。
5. 在 1280×720 下记录 CPU FPS、关键点丢失场景与模型文件大小。
6. 确保 DJI/USB 设备在 Windows 相机列表中可见；不能以 UVC 输出时，交付 MP4 与采集卡方案。

交付验收：标准侧身站立时 17 点位置合理；模型坐标与原图一致；人物离开画面不会产生旧骨架和假计数。

## B：动作算法与集成负责人（你）

分支：`feature/exercise`

当前代码已包含几何、平滑、深蹲/俯卧撑状态机、防误触、错误提示和单元测试。你接下来负责：

1. 在自己电脑运行 `scripts/setup_windows.ps1` 与 `--demo`。
2. 运行核心测试并保留输出截图。
3. 收到 A 的真实 Pose 后，用固定视频调 `Config.h`。
4. 所有共享接口变更先更新 `docs/interfaces.md`，再合并 PR。
5. 每 1–2 天把通过测试的 feature 分支合并到 `develop`。

交付验收：标准深蹲/俯卧撑各 10 次应完整计数；半程、抖动、离场不应增加有效次数。

## C：Qt UI 与交互负责人

分支：`feature/ui`

1. 使用 `--demo` 开发，不等待姿态模型。
2. 主要文件是 `src/ui/MainWindow.cpp`、`include/ui/MainWindow.h`、`resources/dark.qss`。
3. 不在 UI 中写 `if (angle < ...)`；只消费 `ExerciseResult` 和 `TrainingSession`。
4. 测试 1060×680、1440×900 和 1920×1080，Windows 缩放 100%/150%。
5. 检查键盘焦点、空格暂停、ESC 结束、摄像头失败、模型缺失与长文本状态。

交付验收：首页、训练页、结果页可完整切换；视频不拉伸；重要数字远距离可读；窗口缩小不遮住核心按钮。

## D：训练统计与测试负责人

分支：`feature/training`

1. 验证开始、暂停、恢复、结束后的有效时间。
2. 检查 CSV 首次写入表头、后续追加、路径不可写时的提示。
3. 组织 `docs/test_plan.md` 的视频；每种至少 3 组，保留预期和程序输出。
4. 汇总准确率、漏计、误计与失败原因，反馈给 B 调阈值。
5. 第 17 天后只维护测试、Bug 表、Release 和答辩数据。

交付验收：相同测试视频重复运行结果一致；暂停时间不计入训练；CSV 可被 Excel 正常打开。

## 集成顺序

1. 先合并 `common` 接口与 core tests。
2. 合并 A 的 `PoseDetector`，用固定 MP4 验证。
3. 合并 C 的 UI，先用 Demo，再换真实 Pose。
4. 合并 D 的统计与测试数据。
5. 在 `develop` 运行：核心测试 → Demo → MP4 → 摄像头。
6. 全部通过后合并 `main` 并运行 `package_windows.ps1`。
