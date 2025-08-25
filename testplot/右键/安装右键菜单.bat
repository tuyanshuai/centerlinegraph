@echo off
chcp 65001 >nul
echo ================================
echo       安装右键菜单 - 一键作图
echo ================================
echo.
echo 此脚本将为txt文件和所有文件添加右键菜单"一键作图"
echo.
echo 请确认：
echo 1. 当前目录包含txtplotter.exe程序
echo 2. 你有管理员权限（建议以管理员身份运行）
echo.

REM 获取脚本目录
set "SCRIPT_DIR=%~dp0"
set "EXE_PATH=%SCRIPT_DIR%\txtplotter.exe"

REM 检查必需文件
if not exist "%EXE_PATH%" (
    echo [错误] 找不到txtplotter.exe程序！
    echo 路径：%EXE_PATH%
    echo.
    echo 请先编译项目或确保exe文件存在。
    pause
    exit /b 1
)

echo [检查] txtplotter.exe - 找到 ✓
echo.

pause

echo 正在安装右键菜单...
echo.

REM 直接使用reg命令添加注册表项
echo [1/4] 添加txt文件右键菜单项...
reg add "HKEY_CLASSES_ROOT\.txt\shell\一键作图" /ve /d "一键作图" /f >nul 2>&1
if %errorlevel% neq 0 (
    echo [失败] 无法添加txt文件菜单项！请以管理员身份运行。
    goto :install_failed
)

echo [2/4] 设置txt文件菜单图标...
reg add "HKEY_CLASSES_ROOT\.txt\shell\一键作图" /v "Icon" /d "%EXE_PATH%,0" /f >nul 2>&1

echo [3/4] 添加txt文件命令...
reg add "HKEY_CLASSES_ROOT\.txt\shell\一键作图\command" /ve /d "\"%EXE_PATH%\" --file \"%%1\"" /f >nul 2>&1
if %errorlevel% neq 0 (
    echo [失败] 无法添加txt文件命令！
    goto :install_failed
)

echo [4/4] 添加所有文件右键菜单...
reg add "HKEY_CLASSES_ROOT\*\shell\一键作图" /ve /d "一键作图" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\*\shell\一键作图" /v "Icon" /d "%EXE_PATH%,0" /f >nul 2>&1
reg add "HKEY_CLASSES_ROOT\*\shell\一键作图\command" /ve /d "\"%EXE_PATH%\" --file \"%%1\"" /f >nul 2>&1

echo.
echo [成功] 右键菜单安装完成！
echo.
echo 🎉 现在你可以：
echo ✓ 右键任意txt文件，选择"一键作图"
echo ✓ 右键任意文件，选择"一键作图"（程序会尝试读取数据）
echo.
echo 💡 如需卸载，请运行"卸载右键菜单.bat"
goto :install_end

:install_failed
echo.
echo ❌ [失败] 安装失败！
echo.
echo 可能的原因：
echo 1. 没有管理员权限 - 请右键此文件选择"以管理员身份运行"
echo 2. 系统限制了注册表修改
echo.

:install_end

echo.
pause