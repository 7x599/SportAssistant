param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$LocalVcpkg = Join-Path $ProjectRoot ".tools\vcpkg"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "未找到 CMake。请安装 CMake 3.24+，并重新打开 PowerShell。"
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "未找到 Git。请安装 Git for Windows。"
}
if (-not (Test-Path (Join-Path $LocalVcpkg "vcpkg.exe"))) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $LocalVcpkg) | Out-Null
    git clone https://github.com/microsoft/vcpkg.git $LocalVcpkg
    & (Join-Path $LocalVcpkg "bootstrap-vcpkg.bat") -disableMetrics
}

$env:VCPKG_ROOT = $LocalVcpkg
Push-Location $ProjectRoot
try {
    cmake --preset windows-release
    cmake --build --preset windows-release --config $Configuration --parallel
    if (-not $SkipTests) {
        ctest --preset windows-release -C $Configuration
    }
    Write-Host "`n构建完成：build\windows-release\$Configuration\SportAssistant.exe" -ForegroundColor Green
    Write-Host "演示模式：build\windows-release\$Configuration\SportAssistant.exe --demo"
} finally {
    Pop-Location
}
