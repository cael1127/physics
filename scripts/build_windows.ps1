param(
  [ValidateSet("Debug","Release")]
  [string]$Config = "Release",
  [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

Write-Host "== FullPhysicsC Windows build =="
Write-Host "Config: $Config"
Write-Host "BuildDir: $BuildDir"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Write-Error "CMake not found on PATH. Install CMake (https://cmake.org/download/) and reopen your terminal."
}

cmake -S . -B $BuildDir -A x64
cmake --build $BuildDir --config $Config

Write-Host "Built:"
Write-Host " - $BuildDir\$Config\fullphysics_app.exe"
Write-Host " - $BuildDir\$Config\fullphysics_tests.exe"

