r"""Download and export the standard Ultralytics pose model to ONNX.

Run from the project root after installing Python 3.10+:
    py -m venv .venv-model
    .\.venv-model\Scripts\python -m pip install ultralytics onnx onnxslim
    .\.venv-model\Scripts\python tools\export_pose_model.py
"""

from pathlib import Path

from ultralytics import YOLO


def main() -> None:
    project_root = Path(__file__).resolve().parents[1]
    models_dir = project_root / "models"
    models_dir.mkdir(parents=True, exist_ok=True)

    model = YOLO("yolo11n-pose.pt")
    exported = Path(
        model.export(format="onnx", imgsz=640, simplify=True, dynamic=False, opset=17)
    )
    destination = models_dir / "yolo11n-pose.onnx"
    destination.write_bytes(exported.read_bytes())
    print(f"Model ready: {destination}")


if __name__ == "__main__":
    main()
