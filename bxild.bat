@echo off
setlocal enabledelayedexpansion

:: -------------------------------
:: Parse command line argument
:: -------------------------------
set "Target=debug"
if not "%~1"=="" set "Target=%~1"

:: -------------------------------
:: Global settings
:: -------------------------------
set CC=cl
set CFLAGS=-nologo -GR- -Gm- -MT -Zi -EHsc -EHa- -W4 -WX -Fm -DCANVXS=1

set LINKER_FLAGS=/link -incremental:no -opt:ref

set DISABLED_WARNINGS=-wd4100 -wd4201 -wd4996
set DEBUG_FLAGS=-Oi -DDEBUG=1

set RELEASE_FLAGS=-Oi -O2

set SRC_DIR=..\..\src
set EXE_SRC=%SRC_DIR%\windows.cpp
set DLL_SRC=%SRC_DIR%\canvas.cpp

set OUTDIR=build
set DBGDIR=%OUTDIR%\debug
set RELDIR=%OUTDIR%\release

set DEBUGGER=raddbg

set NAME=Canvxs

:: -------------------------------
:: Dispatcher
:: -------------------------------
if /i "%Target%"=="debug" goto :Build-Debug
if /i "%Target%"=="release" goto :Build-Release
if /i "%Target%"=="run" goto :RunProj
if /i "%Target%"=="dbg" goto :RunInDebugger
if /i "%Target%"=="clean" goto :Clean

echo Unknown target: %Target%
echo Available targets: debug, release, run, dbg, clean
exit /b 1

:: -------------------------------
:: Build targets
:: -------------------------------
:Build-Debug
call :Folders
pushd "%DBGDIR%"
%CC% %CFLAGS% %DISABLED_WARNINGS% %DEBUG_FLAGS% /Fe%NAME%.exe %EXE_SRC% %LINKER_FLAGS%
%CC% %CFLAGS% %DISABLED_WARNINGS% %DEBUG_FLAGS% %DLL_SRC% /LD %LINKER_FLAGS% /EXPORT:CanvasUpdateAndRender
popd
goto :eof

:Build-Release
call :Folders
pushd "%RELDIR%"
%CC% %CFLAGS% %RELEASE_FLAGS% /Fe%NAME%.exe %EXE_SRC% %LINKER_FLAGS%
popd
goto :eof

:RunProj
call :Build-Debug
"%DBGDIR%\%NAME%.exe"
goto :eof

:RunInDebugger
call :Build-Debug
pushd "%DBGDIR%"
%DEBUGGER% "%NAME%.exe"
popd
goto :eof

:Clean
if exist "%OUTDIR%" (
    rmdir /s /q "%OUTDIR%"
    echo Cleaned build directory.
) else (
    echo Nothing to clean.
)
goto :eof

:: -------------------------------
:: Utility functions
:: -------------------------------
:Folders
if not exist "%DBGDIR%" mkdir "%DBGDIR%"
if not exist "%RELDIR%" mkdir "%RELDIR%"
goto :eof
