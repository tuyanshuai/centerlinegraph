@echo off
chcp 65001 >nul
echo ================================
echo       卸载右键菜单 - 一键作图
echo ================================
echo.
echo 此脚本将移除txt文件和所有文件的右键菜单"一键作图"
echo.
echo 确认要卸载吗？
pause

echo 正在卸载右键菜单...
echo.

REM 直接使用reg命令删除注册表项
echo [1/2] 删除txt文件右键菜单...
reg delete "HKEY_CLASSES_ROOT\.txt\shell\一键作图" /f >nul 2>&1
if %errorlevel% equ 0 (
    echo ✓ txt文件右键菜单已删除
) else (
    echo ! txt文件右键菜单删除失败（可能不存在）
)

echo [2/2] 删除所有文件右键菜单...
reg delete "HKEY_CLASSES_ROOT\*\shell\一键作图" /f >nul 2>&1
if %errorlevel% equ 0 (
    echo ✓ 所有文件右键菜单已删除
) else (
    echo ! 所有文件右键菜单删除失败（可能不存在）
)

echo.
echo [完成] 右键菜单卸载完成！
echo.
echo 🗑️ 右键菜单"一键作图"已从系统中移除。
echo 💡 如需重新安装，请运行"安装右键菜单.bat"

echo.
pause