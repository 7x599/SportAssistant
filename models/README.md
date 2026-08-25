# Pose model

The application expects `models/yolo11n-pose.onnx` by default. Generate it on Windows from the project root:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\export_model.ps1
```

The vision module expects a single-person Ultralytics COCO pose output with 17 keypoints and either `[1, 56, N]` or `[1, N, 56]` layout. Replace the implementation behind `PoseDetector` if member A chooses another model; do not change `Pose` or the analyzer interfaces.
