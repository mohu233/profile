@echo off
setlocal enabledelayedexpansion
echo ============================================
echo   ProfilePower Build ^& Package Script
echo ============================================
echo.

set "PROJECT_DIR=E:\Documents\untitled3"
set "BUILD_DIR=%PROJECT_DIR%\build\Desktop_Qt_6_11_1_MinGW_64_bit-Debug"
set "QT_DIR=E:\Qt\6.11.1\mingw_64"
set "MINGW_DIR=E:\Qt\Tools\mingw1310_64"
set "PKG_DIR=E:\Documents\profile_package"
set "OUTPUT=E:\Documents\ProfilePower_Setup.exe"
set "NSIS=C:\Program Files (x86)\NSIS\makensis.exe"

echo [1/6] Setting up environment...
set "PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%"

echo [2/6] Building Release...
cd /d "%BUILD_DIR%"
mingw32-make clean >nul 2>&1
mingw32-make release -j4
if errorlevel 1 (echo ERROR: Build failed! && pause && exit /b 1)
echo     Release build OK.

echo [3/6] Deploying with windeployqt...
if exist "%PKG_DIR%" rmdir /s /q "%PKG_DIR%"
mkdir "%PKG_DIR%"
copy "%BUILD_DIR%\release\untitled3.exe" "%PKG_DIR%\profile.exe" >nul
windeployqt --release --no-translations "%PKG_DIR%\profile.exe" >nul 2>&1
echo     windeployqt OK.

echo [4/6] Copying proxy_relay.py...
copy "%PROJECT_DIR%\proxy_relay.py" "%PKG_DIR%\proxy_relay.py" >nul
echo     Copy OK.

echo [5/6] Building installer with NSIS...
powershell -NoProfile -ExecutionPolicy Bypass -File "%PROJECT_DIR%\write_nsis.ps1" "%PKG_DIR%" "%OUTPUT%"
if errorlevel 1 (echo ERROR: PS1 failed! && pause && exit /b 1)
"%NSIS%" "%PKG_DIR%\installer.nsi"
if errorlevel 1 (echo ERROR: NSIS failed! && pause && exit /b 1)
echo     Installer built OK.

echo [6/6] Cleaning up...
rmdir /s /q "%PKG_DIR%" 2>nul

echo ============================================
echo   DONE! Installer: %OUTPUT%
echo ============================================
