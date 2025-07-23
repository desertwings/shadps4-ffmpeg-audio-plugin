@echo off
REM ShadPS4 FFmpeg Audio Integration - Copy to Target Directory Script
REM This script copies the entire project to D:\moddedchadps4\shadPS4\

echo ========================================
echo ShadPS4 FFmpeg Audio Integration
echo Copy to Target Directory Script
echo ========================================

REM Check if we're in the right directory
if not exist "CMakeLists.txt" (
    echo Error: CMakeLists.txt not found. Please run this script from the project root.
    pause
    exit /b 1
)

REM Set source and target directories
set SOURCE_DIR=%CD%
set TARGET_DIR=D:\moddedchadps4\shadPS4

echo Source Directory: %SOURCE_DIR%
echo Target Directory: %TARGET_DIR%
echo.

REM Create target directory structure
echo Creating target directory structure...
if not exist "D:\moddedchadps4" (
    mkdir "D:\moddedchadps4"
    echo Created: D:\moddedchadps4
)

if not exist "%TARGET_DIR%" (
    mkdir "%TARGET_DIR%"
    echo Created: %TARGET_DIR%
)

REM Copy all files and directories
echo.
echo Copying project files...
echo.

REM Copy root files
echo Copying root files...
copy "%SOURCE_DIR%\CMakeLists.txt" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\README.md" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\IMPLEMENTATION_SUMMARY.md" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\DOWNLOAD_SETUP_GUIDE.md" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\COPY_PROJECT_GUIDE.md" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\build.bat" "%TARGET_DIR%\" >nul
copy "%SOURCE_DIR%\copy_to_target.bat" "%TARGET_DIR%\" >nul

REM Copy directories with all contents
echo Copying src directory...
xcopy "%SOURCE_DIR%\src" "%TARGET_DIR%\src" /E /I /H /Y >nul

echo Copying externals directory...
xcopy "%SOURCE_DIR%\externals" "%TARGET_DIR%\externals" /E /I /H /Y >nul

echo Copying sys_modules directory...
xcopy "%SOURCE_DIR%\sys_modules" "%TARGET_DIR%\sys_modules" /E /I /H /Y >nul

echo.
echo ========================================
echo Copy completed successfully!
echo ========================================
echo.

REM Verify copied files
echo Verifying copied files:
if exist "%TARGET_DIR%\CMakeLists.txt" (
    echo ✓ CMakeLists.txt
) else (
    echo ✗ CMakeLists.txt - MISSING
)

if exist "%TARGET_DIR%\src\main.cpp" (
    echo ✓ src\main.cpp
) else (
    echo ✗ src\main.cpp - MISSING
)

if exist "%TARGET_DIR%\src\core\libraries\audio\OrbisAudioDecoder.cpp" (
    echo ✓ src\core\libraries\audio\OrbisAudioDecoder.cpp
) else (
    echo ✗ src\core\libraries\audio\OrbisAudioDecoder.cpp - MISSING
)

if exist "%TARGET_DIR%\src\core\libraries\ajm\plugin_interface.h" (
    echo ✓ src\core\libraries\ajm\plugin_interface.h
) else (
    echo ✗ src\core\libraries\ajm\plugin_interface.h - MISSING
)

echo.
echo Project copied to: %TARGET_DIR%
echo.
echo NEXT STEPS:
echo 1. Download FFmpeg libraries from: https://github.com/BtbN/FFmpeg-Builds/releases
echo 2. Extract FFmpeg to: %TARGET_DIR%\externals\ffmpeg-core\
echo 3. Run: %TARGET_DIR%\build.bat
echo.
echo For detailed setup instructions, see:
echo   - %TARGET_DIR%\DOWNLOAD_SETUP_GUIDE.md
echo   - %TARGET_DIR%\COPY_PROJECT_GUIDE.md
echo.

pause
