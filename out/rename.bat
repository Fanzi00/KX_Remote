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

:: 解析机型宏：从 _Device_config.h 提取 USER_TYPE 定义值（如 KXCAN_GLGJT312E_L3X）
:: 解析失败不中止构建，仅警告并退回原命名
set "userType="
set "hdr=%~dp0..\User\_Device_config.h"
for /f "tokens=3" %%t in ('findstr /C:"#define USER_TYPE" "%hdr%"') do set "userType=%%t"
if defined userType (
    echo [获取机型] USER_TYPE=!userType!
) else (
    echo [rename.bat警告] 未解析到USER_TYPE，固件名将不带机型后缀
)

:: 目录规划：..\..\ = Project根；工程内out 保留本工程最新bin，Project\out 存放最新固件（发射/接收各一个），Project\bin 每机型归档最新一个（同机型不同时间戳只留最新）
set "projRoot=%~dp0..\.."
set "pubOutDir=!projRoot!\out"
set "pubBinDir=!projRoot!\bin"
if not exist "!pubOutDir!" mkdir "!pubOutDir!"
if not exist "!pubBinDir!" mkdir "!pubBinDir!"

echo [清理旧固件] 匹配前缀：!baseName!
:: 工程内out与公共out目录：删除所有同前缀旧bin（只保留最新一个）
del /f /q "!pubOutDir!\!baseName!_*.bin" 2>nul
del /f /q "!binFolder!!baseName!_*.bin" 2>nul
:: 归档bin目录：仅删除同前缀同机型的旧bin（其他机型的bin保留），机型未解析到时退回清全部同前缀
if defined userType (
    del /f /q "!pubBinDir!\!baseName!_!userType!_*.bin" 2>nul
) else (
    del /f /q "!pubBinDir!\!baseName!_*.bin" 2>nul
)
echo [清理旧固件] 已删除旧时间戳BIN文件

:: 生成时间戳 YYYYMMDD_HHMMSS
set Y=%date:~0,4%
set M=%date:~5,2%
set D=%date:~8,2%
set H=%time:~0,2%
set Mi=%time:~3,2%
set S=%time:~6,2%
if "%H%"==" " set H=0%time:~1,1%
set timeStamp=%Y%%M%%D%_%H%%Mi%%S%

:: 拼接新名：前缀_机型_时间戳（未解析到机型时退回 前缀_时间戳）
if defined userType (
    set newFileName=!baseName!_!userType!_!timeStamp!!ext!
) else (
    set newFileName=!baseName!_!timeStamp!!ext!
)
echo [重命名] 原始文件：!baseName!!ext!
echo [重命名] 新固件名称：!newFileName!

:: 工程内out目录原地改名保留本工程bin，再各拷贝一份到公共out（最新固件）和bin目录（按机型归档）
ren "%src%" "!newFileName!" >nul

if errorlevel 1 (
    echo [rename.bat错误] 重命名失败，文件被占用或无写入权限
    exit /b 1
)
copy /y "!binFolder!!newFileName!" "!pubOutDir!" >nul
if errorlevel 1 (
    echo [rename.bat错误] 拷贝到公共out目录失败
    exit /b 1
)
copy /y "!binFolder!!newFileName!" "!pubBinDir!" >nul
if errorlevel 1 (
    echo [rename.bat错误] 拷贝到归档bin目录失败
    exit /b 1
)
echo [rename.bat完成] 本工程固件：!binFolder!!newFileName!
echo [rename.bat完成] 最新固件：!pubOutDir!\!newFileName!
echo [rename.bat完成] 已归档到：!pubBinDir!
exit /b 0
