@echo off
echo ==========================================
echo  Rebuilding Qt Project with Multi-Series Support
echo ==========================================

REM Setup Visual Studio environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Clean previous build
echo.
echo Cleaning previous build...
nmake clean

REM Generate new Makefile
echo.
echo Generating new Makefile...
qmake

REM Compile project
echo.
echo Compiling project with multi-series support...
nmake

echo.
echo Rebuild completed!
echo Multi-series plotting with legends is now available!
pause