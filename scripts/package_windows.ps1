param([ValidateSet("Release", "Debug")][string]$Configuration = "Release")

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Exe = Join-Path $ProjectRoot "build\windows-release\$Configuration\SportAssistant.exe"
$Destination = Join-Path $ProjectRoot "dist\SportAssistant"
$VcpkgInstalled = Join-Path $ProjectRoot "build\windows-release\vcpkg_installed\x64-windows"

if (-not (Test-Path $Exe)) { throw "请先运行 scripts\build_windows.ps1。" }
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
Copy-Item $Exe $Destination -Force

$WinDeployQt = Join-Path $VcpkgInstalled "tools\Qt6\bin\windeployqt.exe"
if (-not (Test-Path $WinDeployQt)) { throw "未找到 windeployqt：$WinDeployQt" }
$QtMode = if ($Configuration -eq "Debug") { "--debug" } else { "--release" }
& $WinDeployQt $QtMode --no-translations --compiler-runtime (Join-Path $Destination "SportAssistant.exe")

$DependencyBin = if ($Configuration -eq "Debug") {
    Join-Path $VcpkgInstalled "debug\bin"
} else {
    Join-Path $VcpkgInstalled "bin"
}
Get-ChildItem $DependencyBin -Filter "*.dll" | Copy-Item -Destination $Destination -Force
if (Test-Path (Join-Path $ProjectRoot "models\yolo11n-pose.onnx")) {
    New-Item -ItemType Directory -Force -Path (Join-Path $Destination "models") | Out-Null
    Copy-Item (Join-Path $ProjectRoot "models\yolo11n-pose.onnx") (Join-Path $Destination "models") -Force
}
Write-Host "可分发目录已生成：dist\SportAssistant" -ForegroundColor Green
