@echo off
REM ShadPS4 FFmpeg Audio Integration Build Script
REM This script automates the build process for Windows

echo ========================================
echo ShadPS4 FFmpeg Audio Integration Build
echo ========================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

REM Configure with CMake
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo Error: CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if errorlevel 1 (
    echo Error: Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Output files:
echo   - sys_modules\libSceM4aacDec.dll
echo   - sys_modules\libSceAjm.dll
echo   - Release\shadps4.exe
echo.
echo To test the audio system, run:
echo   Release\shadps4.exe
echo.

REM Check if output files exist
if exist "sys_modules\libSceM4aacDec.dll" (
    echo ✓ libSceM4aacDec.dll built successfully
) else (
    echo ✗ libSceM4aacDec.dll not found
)

if exist "sys_modules\libSceAjm.dll" (
    echo ✓ libSceAjm.dll built successfully
) else (
    echo ✗ libSceAjm.dll not found
)

if exist "Release\shadps4.exe" (
    echo ✓ shadps4.exe built successfully
) else (
    echo ✗ shadps4.exe not found
)

echo.
echo Note: Ensure FFmpeg libraries are available in:
echo   externals\ffmpeg-core\ffmpeg-YYYY-MM-DD-^<hash^>-full_build\
echo.

pause
