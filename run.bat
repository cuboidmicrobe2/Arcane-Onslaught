@echo off
cd /d "%~dp0"

set "EXE_PATH=build\Debug\ArcaneOnslaught.exe"

if not exist "%EXE_PATH%" (
    echo Could not find %EXE_PATH%
    echo Build the project first, then run this script again.
    exit /b 1
)

"%EXE_PATH%"
