$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Python = Get-Command py -ErrorAction SilentlyContinue
if (-not $Python) { throw "未找到 Python Launcher。请安装 Python 3.10+。" }

Push-Location $ProjectRoot
try {
    if (-not (Test-Path ".venv-model")) { py -3 -m venv .venv-model }
    & ".\.venv-model\Scripts\python.exe" -m pip install --upgrade pip
    & ".\.venv-model\Scripts\python.exe" -m pip install ultralytics onnx onnxslim
    & ".\.venv-model\Scripts\python.exe" "tools\export_pose_model.py"
} finally {
    Pop-Location
}
