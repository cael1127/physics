# Configure (if needed), build, and launch fullphysics_app.exe (Win32 demo).
# Requires: CMake + MSVC (run install_prerequisites.cmd once if missing).

param(
  [switch]$RunOnly,
  [switch]$Clean
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path $PSScriptRoot -Parent
Set-Location $RepoRoot

function Find-VcVars64 {
  $candidates = @(
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
  )
  foreach ($p in $candidates) {
    if (Test-Path $p) { return $p }
  }
  return $null
}

function Find-BuiltExe {
  $candidates = @(
    (Join-Path $RepoRoot "build\Release\fullphysics_app.exe"),
    (Join-Path $RepoRoot "build\Debug\fullphysics_app.exe"),
    (Join-Path $RepoRoot "build\fullphysics_app.exe")
  )
  foreach ($p in $candidates) {
    if (Test-Path $p) { return $p }
  }
  return $null
}

function Invoke-WithVcEnv([string]$InnerCmd) {
  $vc = Find-VcVars64
  if (-not $vc) {
    Write-Error "MSVC environment not found (vcvars64.bat). Install C++ build tools: run install_prerequisites.cmd from the repo root."
  }
  $repo = $RepoRoot
  # Route cmd stdout/stderr directly to host so this function returns only the exit code.
  # Without this, PowerShell captures build log lines as function output and callers see a non-int array.
  cmd.exe /c "`"$vc`" && cd /d `"$repo`" && $InnerCmd" | Out-Host
  $exitCode = $LASTEXITCODE
  if ($null -eq $exitCode) { return 0 }
  return [int]$exitCode
}

function Get-CMakeGeneratorFromCache {
  $cachePath = Join-Path $RepoRoot "build\CMakeCache.txt"
  if (-not (Test-Path $cachePath)) { return $null }
  foreach ($line in Get-Content -Path $cachePath -ErrorAction SilentlyContinue) {
    if ($line -match '^CMAKE_GENERATOR:') {
      $parts = $line -split '=', 2
      if ($parts.Count -ge 2) { return $parts[1].Trim() }
    }
  }
  return $null
}

Write-Host "=== FullPhysicsC: build + run ===" -ForegroundColor Cyan
Write-Host "Repo: $RepoRoot"

$existing = Find-BuiltExe
if ($RunOnly) {
  if (-not $existing) {
    Write-Error "RunOnly specified but no fullphysics_app.exe found under build\. Build once without -RunOnly."
  }
  Write-Host "Starting: $existing" -ForegroundColor Green
  Start-Process -FilePath $existing -WorkingDirectory $RepoRoot
  exit 0
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
  Write-Error "CMake not on PATH. Run install_prerequisites.cmd, then open a new terminal."
}

$buildDir = Join-Path $RepoRoot "build"
$cache = Join-Path $buildDir "CMakeCache.txt"

if ($Clean -and (Test-Path $buildDir)) {
  Write-Host "Removing build\ (--Clean)..." -ForegroundColor Yellow
  Remove-Item -Recurse -Force $buildDir
}

if (-not (Test-Path $cache)) {
  Write-Host "Configuring (NMake, Release)..." -ForegroundColor Cyan
  $code = Invoke-WithVcEnv 'cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release'
  if ($null -eq $code) { $code = 0 }
  if ($code -ne 0) {
    Write-Error "CMake configure failed (exit $code). Try: run_fullphysics.cmd -Clean"
  }
} else {
  $gen = Get-CMakeGeneratorFromCache
  Write-Host "Existing CMake cache (generator: $gen)" -ForegroundColor Cyan

  if ($gen -match 'NMake') {
    $makefile = Join-Path $buildDir "Makefile"
    if (-not (Test-Path $makefile)) {
      Write-Host "NMake cache is incomplete (no Makefile). Reconfiguring..." -ForegroundColor Yellow
      $code = Invoke-WithVcEnv 'cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release'
      if ($null -eq $code) { $code = 0 }
      if ($code -ne 0) {
        Write-Error "CMake reconfigure failed (exit $code). Try: run_fullphysics.cmd -Clean"
      }
    }
  }
}

$gen2 = Get-CMakeGeneratorFromCache
if ($gen2 -match 'Visual Studio') {
  Write-Host "Building (Visual Studio, Release)..." -ForegroundColor Cyan
  $code = Invoke-WithVcEnv 'cmake --build build --config Release'
} elseif ($gen2 -match 'NMake') {
  Write-Host "Building (NMake, Release)..." -ForegroundColor Cyan
  $code = Invoke-WithVcEnv 'cmake --build build'
} else {
  Write-Host "Building (generic)..." -ForegroundColor Cyan
  $code = Invoke-WithVcEnv 'cmake --build build --config Release'
  if (($null -eq $code) -or ($code -ne 0)) {
    $code = Invoke-WithVcEnv 'cmake --build build'
  }
}

if ($null -eq $code) { $code = 0 }
if ($code -ne 0) {
  Write-Error "Build failed (exit $code). If build\ is corrupted, run: run_fullphysics.cmd -Clean"
}

$exe = Find-BuiltExe
if (-not $exe) {
  Write-Error "Build reported success but fullphysics_app.exe was not found under build\."
}

Write-Host "Starting: $exe" -ForegroundColor Green
Start-Process -FilePath $exe -WorkingDirectory $RepoRoot
