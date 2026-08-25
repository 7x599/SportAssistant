# 四人统一接口

## A → B：Pose

唯一姿态数据结构在 `include/common/Pose.h`。关键点顺序严格采用 COCO 17 点，A 只输出每帧 `Pose`，不在视觉模块中计数。

```cpp
Pose pose = detector.detect(frame, timestampSeconds);
ExerciseResult result = analyzer.update(pose);
```

关键约束：

- 坐标必须映射回原始视频像素坐标；
- 置信度范围为 `[0, 1]`；
- `timestampSeconds` 单调递增；
- 未检测到人时返回低置信度空 `Pose`，禁止复用旧姿态伪装成新帧。

## B → C/D：ExerciseResult

唯一结果结构在 `include/common/ExerciseResult.h`：

- `validCount` / `invalidCount`：累计次数；
- `mainAngle`：深蹲膝角或俯卧撑肘角；
- `bodyAngle`：身体线或辅助角度；
- `eventOccurred`：本帧结束了一次动作尝试；
- `eventValid`：该次尝试是否有效；
- `phase`：当前状态机相位；
- `message`：可直接展示的中文反馈。

C 只显示，不再判断角度；D 只统计，不重复定义次数。

## D → C：TrainingSession

UI 只调用 `start/pause/resume/finish/update`，读取目标、时间和完成率。CSV 写入失败应提示用户，但不得导致训练程序崩溃。

## 共享头文件规则

`include/common/` 的改动必须在 PR 说明中列出影响成员。不得新增同义结构，例如 `HumanPose`、`UIResult`、`SessionData`。
