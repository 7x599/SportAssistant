$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $ProjectRoot
try {
    if (-not (Test-Path ".git")) {
        $Email = git config --global user.email
        $Name = git config --global user.name
        if (-not $Email -or -not $Name) {
            throw "请先设置你自己的 Git 身份：git config --global user.name '你的名字'；git config --global user.email '你的邮箱'"
        }
        git init -b main
        git add .
        git commit -m "chore: initialize SportAssistant"
    }
    git branch develop 2>$null
    git branch feature/pose 2>$null
    git branch feature/exercise 2>$null
    git branch feature/ui 2>$null
    git branch feature/training 2>$null
    git branch --list
} finally {
    Pop-Location
}
