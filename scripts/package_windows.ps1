param(
  [ValidateSet("Release","Debug")]
  [string]$Config = "Release",
  [string]$BuildDir = "build",
  [string]$OutDir = "dist\\FullPhysicsC-win64"
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$exe = Join-Path $BuildDir "$Config\\fullphysics_app.exe"
if (-not (Test-Path $exe)) {
  Write-Error "Missing $exe. Run scripts\\build_windows.ps1 first."
}

Copy-Item $exe (Join-Path $OutDir "fullphysics_app.exe") -Force
Copy-Item "README.md" (Join-Path $OutDir "README.md") -Force

Write-Host "Packaged to $OutDir"

