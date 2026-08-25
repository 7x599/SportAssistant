# Verification record

## Completed in the delivery environment

- C++ compiler: GCC 13.3, C++17.
- Warning flags: `-Wall -Wextra -Wpedantic`.
- Compiled modules: geometry, pose smoothing, squat analyzer, push-up analyzer, training session, CSV record.
- Deterministic test result: `All SportAssistant core tests passed.`
- Python model-export helper: syntax compilation passed.

The tests cover a valid squat, partial squat rejection, valid push-up, bent-body push-up rejection, missing-pose safety, angle geometry, counts, and completion rate.

## Must be completed on the Windows target

The delivery container does not include CMake, Qt or OpenCV, so the Qt/OpenCV executable cannot be rendered here. Run:

```powershell
.\scripts\setup_windows.ps1
.\scripts\package_windows.ps1
```

Then verify in this order:

1. `SportAssistant.exe --demo` at 1060×680 and 1440×900.
2. `SportAssistant.exe --video <known-test.mp4>` with the exported ONNX model.
3. `SportAssistant.exe --camera 0` using the final camera.
4. Pause/resume time, target completion, CSV append, missing-model and missing-camera recovery.
5. Run the packaged folder on a second Windows computer without Visual Studio.

Do not call the release complete until these five checks pass.
