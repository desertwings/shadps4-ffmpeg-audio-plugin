#!/bin/bash
# ShadPS4 ext-ffmpeg-core build script
# This script downloads, patches, and builds FFmpeg for ShadPS4

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FFMPEG_VERSION="5.1.6"
FFMPEG_URL="https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2"
FFMPEG_DIR="ffmpeg-${FFMPEG_VERSION}"
BUILD_DIR="ffmpeg-build"

echo "=========================================="
echo "ShadPS4 ext-ffmpeg-core Build Script"
echo "=========================================="
echo "FFmpeg Version: ${FFMPEG_VERSION}"
echo "Build Directory: ${SCRIPT_DIR}/${BUILD_DIR}"
echo "=========================================="

cd "${SCRIPT_DIR}"

# Clean previous builds
if [ -d "${BUILD_DIR}" ]; then
    echo "Cleaning previous build..."
    rm -rf "${BUILD_DIR}"
fi

if [ -d "${FFMPEG_DIR}" ]; then
    echo "Cleaning previous source..."
    rm -rf "${FFMPEG_DIR}"
fi

# Download FFmpeg source
echo "Downloading FFmpeg ${FFMPEG_VERSION}..."
if [ ! -f "ffmpeg-${FFMPEG_VERSION}.tar.bz2" ]; then
    curl -L -o "ffmpeg-${FFMPEG_VERSION}.tar.bz2" "${FFMPEG_URL}"
fi

# Extract FFmpeg source
echo "Extracting FFmpeg source..."
tar -xjf "ffmpeg-${FFMPEG_VERSION}.tar.bz2"

# Apply ShadPS4 patches
echo "Applying ShadPS4 patches..."
cd "${FFMPEG_DIR}"
if [ -f "../ffmpeg.patch" ]; then
    patch -p1 < "../ffmpeg.patch"
    echo "Applied ffmpeg.patch successfully"
else
    echo "Warning: ffmpeg.patch not found, continuing without patches"
fi

# Configure FFmpeg build
echo "Configuring FFmpeg build..."
./configure \
    --prefix="${SCRIPT_DIR}/${BUILD_DIR}" \
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
    --disable-stripping

# Build FFmpeg
echo "Building FFmpeg..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Install FFmpeg
echo "Installing FFmpeg..."
make install

# Create pkg-config files if they don't exist
echo "Setting up pkg-config files..."
PKG_CONFIG_DIR="${SCRIPT_DIR}/${BUILD_DIR}/lib/pkgconfig"
if [ ! -d "${PKG_CONFIG_DIR}" ]; then
    mkdir -p "${PKG_CONFIG_DIR}"
fi

# Generate libavcodec.pc
cat > "${PKG_CONFIG_DIR}/libavcodec.pc" << EOF
prefix=${SCRIPT_DIR}/${BUILD_DIR}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: libavcodec
Description: FFmpeg codec library
Version: ${FFMPEG_VERSION}
Requires: libavutil
Libs: -L\${libdir} -lavcodec
Cflags: -I\${includedir}
EOF

# Generate libavutil.pc
cat > "${PKG_CONFIG_DIR}/libavutil.pc" << EOF
prefix=${SCRIPT_DIR}/${BUILD_DIR}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: libavutil
Description: FFmpeg utility library
Version: ${FFMPEG_VERSION}
Libs: -L\${libdir} -lavutil
Cflags: -I\${includedir}
EOF

# Generate libswresample.pc
cat > "${PKG_CONFIG_DIR}/libswresample.pc" << EOF
prefix=${SCRIPT_DIR}/${BUILD_DIR}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include

Name: libswresample
Description: FFmpeg audio resampling library
Version: ${FFMPEG_VERSION}
Requires: libavutil
Libs: -L\${libdir} -lswresample
Cflags: -I\${includedir}
EOF

echo "=========================================="
echo "FFmpeg build completed successfully!"
echo "=========================================="
echo "Build location: ${SCRIPT_DIR}/${BUILD_DIR}"
echo "Libraries:"
ls -la "${SCRIPT_DIR}/${BUILD_DIR}/lib/"
echo "Headers:"
ls -la "${SCRIPT_DIR}/${BUILD_DIR}/include/"
echo "=========================================="
echo "To use with ShadPS4, ensure CMake can find:"
echo "  PKG_CONFIG_PATH=${SCRIPT_DIR}/${BUILD_DIR}/lib/pkgconfig"
echo "  LD_LIBRARY_PATH=${SCRIPT_DIR}/${BUILD_DIR}/lib"
echo "=========================================="
