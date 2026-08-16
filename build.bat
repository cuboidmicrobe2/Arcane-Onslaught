@echo off
setlocal
cd /d "%~dp0"

set "BUILD_CONFIG=Debug"
set "CLEAN_BUILD=0"

if /I "%~1"=="clean" set "CLEAN_BUILD=1"
if /I "%~1"=="--clean" set "CLEAN_BUILD=1"
if /I "%~1"=="/clean" set "CLEAN_BUILD=1"
if /I "%~1"=="release" set "BUILD_CONFIG=Release"
if /I "%~1"=="--release" set "BUILD_CONFIG=Release"
if /I "%~1"=="rebuild" set "CLEAN_BUILD=1" & set "BUILD_CONFIG=Debug"
if /I "%~1"=="rebuild-release" set "CLEAN_BUILD=1" & set "BUILD_CONFIG=Release"

if not defined VCPKG_ROOT (
    if exist "E:\vcpkg\vcpkg.exe" set "VCPKG_ROOT=E:\vcpkg"
    if exist "C:\vcpkg\vcpkg.exe" set "VCPKG_ROOT=C:\vcpkg"
)

if not defined VCPKG_ROOT (
    echo VCPKG_ROOT is not set and vcpkg was not found in E:\vcpkg or C:\vcpkg
    exit /b 1
)

set "CMAKE_GENERATOR=Visual Studio 18 2026"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq delims=" %%I in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do set "VS_INSTALL_PATH=%%I"
)

if defined VS_INSTALL_PATH (
    set "HAS_VS17=0"
    if exist "%VS_INSTALL_PATH%\VC\Tools\MSVC" (
        for /d %%D in ("%VS_INSTALL_PATH%\VC\Tools\MSVC\*") do (
            if exist "%%~D\bin\Hostx64\x64\cl.exe" (
                set "HAS_VS17=1"
            )
        )
    )
    if "%HAS_VS17%"=="0" (
        set "CMAKE_GENERATOR=Visual Studio 18 2026"
    )
)

set "CMAKE_BIN="
if exist "C:\Program Files\CMake\bin\cmake.exe" set "CMAKE_BIN=C:\Program Files\CMake\bin\cmake.exe"
if not defined CMAKE_BIN if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" set "CMAKE_BIN=C:\Program Files (x86)\CMake\bin\cmake.exe"
if not defined CMAKE_BIN for /f "delims=" %%I in ('where cmake 2^>nul') do set "CMAKE_BIN=%%I"
if not defined CMAKE_BIN (
    echo CMake was not found in the default install locations or PATH.
    exit /b 1
)

if "%CLEAN_BUILD%"=="1" (
    echo Running clean build: removing CMake cache and fetched deps...
    if exist build\CMakeCache.txt del /q build\CMakeCache.txt
    if exist build\CMakeFiles rmdir /s /q build\CMakeFiles
    if exist build\_deps rmdir /s /q build\_deps
)

if exist build\CMakeCache.txt (
    "%CMAKE_BIN%" -S . -B build -G "%CMAKE_GENERATOR%" -A x64 || exit /b 1
) else (
    "%CMAKE_BIN%" -S . -B build -G "%CMAKE_GENERATOR%" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_PREFIX_PATH="%VCPKG_ROOT%\installed\x64-windows" || exit /b 1
)
"%CMAKE_BIN%" --build build --config %BUILD_CONFIG% || exit /b 1

pushd build\%BUILD_CONFIG%
ArcaneOnslaught.exe
popd
