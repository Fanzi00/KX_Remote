@echo off
chcp 936 >nul
setlocal enabledelayedexpansion

set "src=%~1"

:: 校验入参
if "%src%"=="" (
    echo [rename.bat错误] 未传入BIN文件路径参数
    exit /b 2
)
if not exist "%src%" (
    echo [rename.bat错误] 待重命名BIN文件不存在：%src%
    exit /b 1
)

:: 拆分路径、原始文件名、后缀
for %%f in ("%src%") do (
    set binFolder=%%~dpf
    set baseName=%%~nf
    set ext=%%~xf
)

echo [清理旧固件] 匹配前缀：!baseName!
:: 删除out目录下所有以原始文件名为前缀的bin文件（旧时间戳固件）
del /f /q "!binFolder!!baseName!_*.bin" 2>nul
echo [清理旧固件] 已删除历史同前缀BIN文件

:: 生成时间戳 YYYYMMDD_HHMMSS
set Y=%date:~0,4%
set M=%date:~5,2%
set D=%date:~8,2%
set H=%time:~0,2%
set Mi=%time:~3,2%
set S=%time:~6,2%
if "%H%"==" " set H=0%time:~1,1%
set timeStamp=%Y%%M%%D%_%H%%Mi%%S%

set newFileName=!baseName!_!timeStamp!!ext!
echo [重命名] 原始文件：!baseName!!ext!
echo [重命名] 新固件名称：!newFileName!

:: 原地重命名
ren "%src%" "!newFileName!"

if errorlevel 1 (
    echo [rename.bat错误] 重命名失败，文件被占用或无写入权限
    exit /b 1
) else (
    echo [rename.bat完成] 最新固件生成成功，仅保留当前版本
    exit /b 0
)
