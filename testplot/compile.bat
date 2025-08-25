@echo off
echo ==========================================
echo  Simple Qt Compilation Script
echo ==========================================

REM Setup Visual Studio environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

REM Generate Makefile and compile
echo.
echo Generating Makefile...
qmake

echo.
echo Compiling project...
nmake

echo.
echo Compilation completed!
pause