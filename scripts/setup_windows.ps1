param([switch]$InstallPrerequisites)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Install-WingetPackage {
    param([string]$Id, [string]$Override = "")
    $arguments = @("install", "--id", $Id, "--exact", "--accept-source-agreements", "--accept-package-agreements")
    if ($Override) { $arguments += @("--override", $Override) }
    & winget @arguments
}

$missing = @()
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { $missing += "Git" }
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) { $missing += "CMake" }

if ($InstallPrerequisites) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw "未找到 winget。请先从 Microsoft Store 安装“应用安装程序”。"
    }
    Install-WingetPackage "Git.Git"
    Install-WingetPackage "Kitware.CMake"
    Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
    Write-Host "依赖安装完成。请关闭本窗口，打开“Developer PowerShell for VS 2022”，再运行 scripts\setup_windows.ps1。" -ForegroundColor Yellow
    exit 0
}

if ($missing.Count -gt 0) {
    throw "缺少：$($missing -join ', ')。可运行：powershell -ExecutionPolicy Bypass -File scripts\setup_windows.ps1 -InstallPrerequisites"
}

$ProjectRoot = Split-Path -Parent $PSScriptRoot
& (Join-Path $PSScriptRoot "build_windows.ps1")

$Executable = Join-Path $ProjectRoot "build\windows-release\Release\SportAssistant.exe"
if (Test-Path $Executable) {
    Write-Host "`n启动内置演示模式……" -ForegroundColor Cyan
    & $Executable --demo
}
