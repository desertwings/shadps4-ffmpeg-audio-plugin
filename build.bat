@echo off
REM ShadPS4 FFmpeg Audio Integration Build Script
REM This script automates the build process for Windows using ext-ffmpeg-core

echo ========================================
echo ShadPS4 FFmpeg Audio Integration Build
echo ========================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

REM Check if ext-ffmpeg-core exists
if not exist "ext\ext-ffmpeg-core" (
    echo Error: ext-ffmpeg-core not found!
    echo.
    echo Please set up ext-ffmpeg-core first:
    echo   1. Clone: git clone https://github.com/shadps4-emu/ext-ffmpeg-core.git ext\ext-ffmpeg-core
    echo   2. Or copy the provided ext-ffmpeg-core directory
    echo.
    pause
    exit /b 1
)

REM Check if FFmpeg is built
if not exist "ext\ext-ffmpeg-core\ffmpeg-build" (
    echo FFmpeg not built yet. Building ext-ffmpeg-core...
    echo.
    cd ext\ext-ffmpeg-core
    call build.bat
    if errorlevel 1 (
        echo Error: ext-ffmpeg-core build failed!
        pause
        exit /b 1
    )
    cd ..\..
    echo.
    echo ext-ffmpeg-core build completed!
    echo.
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
    echo.
    echo Possible issues:
    echo   - ext-ffmpeg-core not properly built
    echo   - Visual Studio 2022 not installed
    echo   - CMake not found in PATH
    echo.
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if errorlevel 1 (
    echo Error: Build failed!
    echo.
    echo Check the build output above for specific errors.
    echo Common issues:
    echo   - Missing FFmpeg libraries
    echo   - Compiler errors in source code
    echo   - Linking errors
    echo.
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
echo FFmpeg integration using ext-ffmpeg-core:
echo   Location: ext\ext-ffmpeg-core\ffmpeg-build\
echo   Libraries: %cd%\..\ext\ext-ffmpeg-core\ffmpeg-build\lib\
echo   Headers: %cd%\..\ext\ext-ffmpeg-core\ffmpeg-build\include\
echo.

pause
