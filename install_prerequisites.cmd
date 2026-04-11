@echo off
setlocal
cd /d "%~dp0"
echo Starting prerequisite installer...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\install_prerequisites.ps1"
if errorlevel 1 (
  echo.
  echo Installer exited with an error. Try running this file as Administrator.
  pause
  exit /b 1
)
echo.
pause
