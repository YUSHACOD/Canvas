@echo off
setlocal enabledelayedexpansion

:: Check if cl is already available
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo Environment Setup Is Done
    exit /b 0
)

:: Check if vswhere.exe is available
where vswhere.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo vswhere.exe is available in PATH
) else (
    echo vswhere.exe not found in PATH
    set /p "Ans=Do you want to continue setup by installing vswhere (Y/n): "
    
    if "!Ans!"=="" set "Ans=Y"
    if /i "!Ans!"=="Y" (
        winget install vswhere
    ) else if /i "!Ans!"=="N" (
        echo Exitting...
        exit /b 0
    ) else (
        echo Invalid input
        exit /b 0
    )
)

echo Setting up Developer Environment

:: Get VS installation path
for /f "delims=" %%i in ('vswhere -format json') do set "json_output=%%i"

:: Use vswhere with -property to get installationPath directly
for /f "delims=" %%i in ('vswhere -latest -property installationPath') do set "VS_Install_Path=%%i"

if "!VS_Install_Path!"=="" (
    echo Visual Studio not present in the system
    exit /b 0
)

set "DevEnv_Setup_Suffix=\Common7\Tools\LaunchDevCmd.bat"
set "DevEnv_Setup_Path=!VS_Install_Path!!DevEnv_Setup_Suffix!"
set "DevEnv_Setup_Flags=-SkipAutomaticLocation -Arch amd64"
set "command=!DevEnv_Setup_Path! !DevEnv_Setup_Flags!"

echo Running: !command!
"'!DevEnv_Setup_Path!'"
