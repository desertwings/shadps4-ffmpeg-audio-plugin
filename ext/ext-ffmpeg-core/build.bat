@echo off
REM ShadPS4 ext-ffmpeg-core Windows build script
REM This script downloads, patches, and builds FFmpeg for ShadPS4 on Windows

setlocal enabledelayedexpansion

set FFMPEG_VERSION=5.1.6
set FFMPEG_URL=https://ffmpeg.org/releases/ffmpeg-%FFMPEG_VERSION%.tar.bz2
set FFMPEG_DIR=ffmpeg-%FFMPEG_VERSION%
set BUILD_DIR=ffmpeg-build
set SCRIPT_DIR=%~dp0

echo ==========================================
echo ShadPS4 ext-ffmpeg-core Windows Build
echo ==========================================
echo FFmpeg Version: %FFMPEG_VERSION%
echo Build Directory: %SCRIPT_DIR%%BUILD_DIR%
echo ==========================================

cd /d "%SCRIPT_DIR%"

REM Clean previous builds
if exist "%BUILD_DIR%" (
    echo Cleaning previous build...
    rmdir /s /q "%BUILD_DIR%"
)

if exist "%FFMPEG_DIR%" (
    echo Cleaning previous source...
    rmdir /s /q "%FFMPEG_DIR%"
)

REM Check for required tools
where curl >nul 2>&1
if errorlevel 1 (
    echo Error: curl not found. Please install curl or use Windows 10/11 with built-in curl.
    pause
    exit /b 1
)

where tar >nul 2>&1
if errorlevel 1 (
    echo Error: tar not found. Please install tar or use Windows 10/11 with built-in tar.
    pause
    exit /b 1
)

REM Download FFmpeg source
echo Downloading FFmpeg %FFMPEG_VERSION%...
if not exist "ffmpeg-%FFMPEG_VERSION%.tar.bz2" (
    curl -L -o "ffmpeg-%FFMPEG_VERSION%.tar.bz2" "%FFMPEG_URL%"
    if errorlevel 1 (
        echo Error: Failed to download FFmpeg source
        pause
        exit /b 1
    )
)

REM Extract FFmpeg source
echo Extracting FFmpeg source...
tar -xjf "ffmpeg-%FFMPEG_VERSION%.tar.bz2"
if errorlevel 1 (
    echo Error: Failed to extract FFmpeg source
    pause
    exit /b 1
)

REM Apply ShadPS4 patches
echo Applying ShadPS4 patches...
cd "%FFMPEG_DIR%"
if exist "..\ffmpeg.patch" (
    REM Try to apply patch (requires patch.exe or Git Bash)
    where patch >nul 2>&1
    if not errorlevel 1 (
        patch -p1 < "..\ffmpeg.patch"
        echo Applied ffmpeg.patch successfully
    ) else (
        echo Warning: patch command not found, skipping patch application
        echo Please manually apply patches or install Git for Windows
    )
) else (
    echo Warning: ffmpeg.patch not found, continuing without patches
)

REM Check for MSYS2/MinGW or Visual Studio build environment
echo Checking build environment...
where gcc >nul 2>&1
if not errorlevel 1 (
    echo Found GCC, using MinGW/MSYS2 build
    set BUILD_TYPE=mingw
) else (
    where cl >nul 2>&1
    if not errorlevel 1 (
        echo Found MSVC, using Visual Studio build
        set BUILD_TYPE=msvc
    ) else (
        echo Error: No suitable compiler found
        echo Please install either:
        echo   - MSYS2/MinGW64 (recommended)
        echo   - Visual Studio 2022 with C++ tools
        pause
        exit /b 1
    )
)

REM Configure FFmpeg build based on environment
echo Configuring FFmpeg build...
set PREFIX=%SCRIPT_DIR%%BUILD_DIR%
set PREFIX=%PREFIX:\=/%

if "%BUILD_TYPE%"=="mingw" (
    REM MinGW/MSYS2 build configuration
    bash -c "./configure \
        --prefix='%PREFIX%' \
        --enable-shared \
        --disable-static \
        --disable-doc \
        --disable-programs \
        --disable-network \
        --disable-protocols \
        --enable-protocol=file \
        --disable-devices \
        --disable-filters \
        --enable-filter=aresample \
        --enable-filter=format \
        --enable-filter=channelmap \
        --disable-encoders \
        --enable-encoder=aac \
        --disable-muxers \
        --enable-muxer=mp4 \
        --disable-demuxers \
        --enable-demuxer=aac \
        --enable-demuxer=mov \
        --enable-demuxer=mp4 \
        --disable-decoders \
        --enable-decoder=aac \
        --enable-decoder=aac_latm \
        --disable-parsers \
        --enable-parser=aac \
        --enable-parser=aac_latm \
        --disable-bsfs \
        --enable-bsf=aac_adtstoasc \
        --disable-hwaccels \
        --disable-indevs \
        --disable-outdevs \
        --enable-pic \
        --enable-optimizations \
        --disable-debug \
        --disable-stripping \
        --target-os=mingw32 \
        --arch=x86_64"
) else (
    REM MSVC build configuration
    echo MSVC build not fully implemented in this script
    echo Please use MSYS2/MinGW64 for building FFmpeg
    echo Or manually configure FFmpeg for MSVC
    pause
    exit /b 1
)

if errorlevel 1 (
    echo Error: FFmpeg configuration failed
    pause
    exit /b 1
)

REM Build FFmpeg
echo Building FFmpeg...
if "%BUILD_TYPE%"=="mingw" (
    bash -c "make -j%NUMBER_OF_PROCESSORS%"
) else (
    make -j%NUMBER_OF_PROCESSORS%
)

if errorlevel 1 (
    echo Error: FFmpeg build failed
    pause
    exit /b 1
)

REM Install FFmpeg
echo Installing FFmpeg...
if "%BUILD_TYPE%"=="mingw" (
    bash -c "make install"
) else (
    make install
)

if errorlevel 1 (
    echo Error: FFmpeg installation failed
    pause
    exit /b 1
)

REM Create pkg-config files
echo Setting up pkg-config files...
set PKG_CONFIG_DIR=%SCRIPT_DIR%%BUILD_DIR%\lib\pkgconfig
if not exist "%PKG_CONFIG_DIR%" (
    mkdir "%PKG_CONFIG_DIR%"
)

REM Generate libavcodec.pc
(
echo prefix=%SCRIPT_DIR%%BUILD_DIR%
echo exec_prefix=${prefix}
echo libdir=${exec_prefix}/lib
echo includedir=${prefix}/include
echo.
echo Name: libavcodec
echo Description: FFmpeg codec library
echo Version: %FFMPEG_VERSION%
echo Requires: libavutil
echo Libs: -L${libdir} -lavcodec
echo Cflags: -I${includedir}
) > "%PKG_CONFIG_DIR%\libavcodec.pc"

REM Generate libavutil.pc
(
echo prefix=%SCRIPT_DIR%%BUILD_DIR%
echo exec_prefix=${prefix}
echo libdir=${exec_prefix}/lib
echo includedir=${prefix}/include
echo.
echo Name: libavutil
echo Description: FFmpeg utility library
echo Version: %FFMPEG_VERSION%
echo Libs: -L${libdir} -lavutil
echo Cflags: -I${includedir}
) > "%PKG_CONFIG_DIR%\libavutil.pc"

REM Generate libswresample.pc
(
echo prefix=%SCRIPT_DIR%%BUILD_DIR%
echo exec_prefix=${prefix}
echo libdir=${exec_prefix}/lib
echo includedir=${prefix}/include
echo.
echo Name: libswresample
echo Description: FFmpeg audio resampling library
echo Version: %FFMPEG_VERSION%
echo Requires: libavutil
echo Libs: -L${libdir} -lswresample
echo Cflags: -I${includedir}
) > "%PKG_CONFIG_DIR%\libswresample.pc"

echo ==========================================
echo FFmpeg build completed successfully!
echo ==========================================
echo Build location: %SCRIPT_DIR%%BUILD_DIR%
echo Libraries:
dir "%SCRIPT_DIR%%BUILD_DIR%\lib" /b
echo Headers:
dir "%SCRIPT_DIR%%BUILD_DIR%\include" /b
echo ==========================================
echo To use with ShadPS4, ensure CMake can find:
echo   PKG_CONFIG_PATH=%SCRIPT_DIR%%BUILD_DIR%\lib\pkgconfig
echo   PATH=%SCRIPT_DIR%%BUILD_DIR%\bin;%%PATH%%
echo ==========================================

pause
