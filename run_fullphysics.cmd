@echo off
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\run_app.ps1" %*
if errorlevel 1 (
  echo.
  echo Build or launch failed. Install tools with install_prerequisites.cmd if needed.
  pause
  exit /b 1
)
