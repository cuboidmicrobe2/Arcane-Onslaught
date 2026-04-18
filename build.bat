@echo off
setlocal
cd /d "%~dp0"

set "CLEAN_BUILD=0"
if /I "%~1"=="clean" set "CLEAN_BUILD=1"
if /I "%~1"=="--clean" set "CLEAN_BUILD=1"
if /I "%~1"=="/clean" set "CLEAN_BUILD=1"

if not defined VCPKG_ROOT (
	if exist E:\vcpkg\vcpkg.exe set "VCPKG_ROOT=E:\vcpkg"
	if exist C:\vcpkg\vcpkg.exe set "VCPKG_ROOT=C:\vcpkg"
)

if not defined VCPKG_ROOT (
	echo VCPKG_ROOT is not set and vcpkg was not found in E:\vcpkg or C:\vcpkg
	exit /b 1
)

if "%CLEAN_BUILD%"=="1" (
	echo Running clean build: removing CMake cache and fetched deps...
	if exist build\CMakeCache.txt del /q build\CMakeCache.txt
	if exist build\CMakeFiles rmdir /s /q build\CMakeFiles
	if exist build\_deps rmdir /s /q build\_deps
)

if exist build\CMakeCache.txt (
	cmake -S . -B build -G "Visual Studio 18 2026" -A x64 || exit /b 1
) else (
	cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_PREFIX_PATH="%VCPKG_ROOT%\installed\x64-windows" || exit /b 1
)
cmake --build build --config Debug || exit /b 1

pushd build\Debug
ArcaneOnslaught.exe
popd
