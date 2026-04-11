@echo off
setlocal
cd /d "%~dp0"

set CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe
if not exist "%CSC%" (
  echo Could not find C# compiler at:
  echo   %CSC%
  echo Install .NET Framework developer tools or Visual Studio Build Tools.
  pause
  exit /b 1
)

if not exist "scripts\launcher" mkdir "scripts\launcher"

"%CSC%" /nologo /optimize+ /target:exe /out:"FullPhysicsLauncher.exe" "scripts\launcher\FullPhysicsLauncher.cs"
if errorlevel 1 (
  echo Failed to build FullPhysicsLauncher.exe
  pause
  exit /b 1
)

echo Built: FullPhysicsLauncher.exe
exit /b 0
