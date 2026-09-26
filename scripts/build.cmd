@echo off
setlocal
set "SH09_ROOT=%~dp0.."
set "SH09_CMAKE=cmake"
where cmake >nul 2>nul
if errorlevel 1 (
  set "SH09_CMAKE=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
)
if not defined JUCE_DIR set "JUCE_DIR=%SH09_ROOT%\..\JUCE"
"%SH09_CMAKE%" -S "%SH09_ROOT%" -B "%SH09_ROOT%\build" -G "Visual Studio 18 2026" -A x64 -DJUCE_DIR="%JUCE_DIR%"
if errorlevel 1 exit /b 1
"%SH09_CMAKE%" --build "%SH09_ROOT%\build" --config Release --parallel 8
exit /b %ERRORLEVEL%
