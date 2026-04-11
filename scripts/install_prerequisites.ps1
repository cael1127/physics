# Installs FullPhysicsC Windows build prerequisites via winget (CMake, MSVC Build Tools + C++ workload, Windows SDK).
# Run from an elevated PowerShell if winget prompts for admin (right-click -> Run as administrator).

$ErrorActionPreference = "Stop"

function Write-Info($msg) { Write-Host $msg -ForegroundColor Cyan }
function Write-Warn($msg) { Write-Host $msg -ForegroundColor Yellow }

function Test-CommandExists([string]$Name) {
  return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Refresh-PathFromMachine {
  $machine = [System.Environment]::GetEnvironmentVariable("Path", "Machine")
  $user = [System.Environment]::GetEnvironmentVariable("Path", "User")
  $env:Path = "$machine;$user"
}

function Test-VcVars64Exists {
  $p = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
  if (Test-Path $p) { return $true }
  $p2 = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
  if (Test-Path $p2) { return $true }
  $p3 = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
  if (Test-Path $p3) { return $true }
  return $false
}

Write-Info "=== FullPhysicsC: install prerequisites (winget) ==="
Write-Warn "This may take several minutes and can show UAC prompts for Visual Studio / SDK installers."

if (-not (Test-CommandExists "winget")) {
  Write-Error "winget not found. Install App Installer / Windows Package Manager from the Microsoft Store, or use Windows 11 / updated Windows 10."
}

Refresh-PathFromMachine

function Test-WindowsSdkRcExists {
  $root = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
  if (-not (Test-Path $root)) { return $false }
  $rc = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue | ForEach-Object {
    $cand = Join-Path $_.FullName "x64\rc.exe"
    if (Test-Path $cand) { $cand }
  } | Select-Object -First 1
  return [bool]$rc
}

$needCmake = -not (Test-CommandExists "cmake")
$needSdk = -not (Test-WindowsSdkRcExists)
$needMsvc = -not (Test-VcVars64Exists)

Write-Info "Planned installs: CMake=$(if ($needCmake) { 'yes' } else { 'skip (already on PATH)' })"
Write-Info "                 Windows SDK=$(if ($needSdk) { 'yes' } else { 'skip (rc.exe found)' })"
Write-Info "                 VS 2022 Build Tools (C++)=$(if ($needMsvc) { 'yes' } else { 'skip (vcvars64 found)' })"

$wingetCommon = @(
  "--accept-package-agreements",
  "--accept-source-agreements"
)

if ($needCmake) {
  Write-Info "Installing CMake (Kitware.CMake)..."
  winget install --id Kitware.CMake -e @wingetCommon
  Refresh-PathFromMachine
}

if ($needSdk) {
  Write-Info "Installing Windows 10 SDK (10.0.22621) for rc.exe / mt.exe..."
  winget install --id Microsoft.WindowsSDK.10.0.22621 -e @wingetCommon
  Refresh-PathFromMachine
}

if ($needMsvc) {
  Write-Info "Installing Visual Studio 2022 Build Tools with MSVC (C++ workload)..."
  $override = '--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'
  winget install --id Microsoft.VisualStudio.2022.BuildTools -e @wingetCommon --override $override
  Refresh-PathFromMachine
}

if (-not $needCmake -and -not $needSdk -and -not $needMsvc) {
  Write-Info "All checked prerequisites are already present."
}

Write-Info ""
Write-Info "Done. Close this window, open a new PowerShell or 'x64 Native Tools Command Prompt for VS 2022', then from the repo root:"
Write-Info "  cmake -S . -B build -A x64"
Write-Info "  cmake --build build --config Release"
Write-Info "  .\build\Release\fullphysics_app.exe"
Write-Info ""
Write-Warn "If cmake is still not found, sign out and back in (or reboot) so PATH updates from the installers."
