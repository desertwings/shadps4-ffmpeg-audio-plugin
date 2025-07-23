# ShadPS4 ext-ffmpeg-core

This repository contains the FFmpeg integration for ShadPS4 emulator, following the official ShadPS4 pattern for external dependencies.

## Overview

This module provides:
- **FFmpeg 5.1.6** with ShadPS4-specific patches
- **M4AAC audio decoding** support for PlayStation 4 games
- **Optimized build configuration** for emulator use
- **Cross-platform build scripts** for Windows, Linux, and macOS

## Quick Start

### Prerequisites

**Linux/macOS:**
- GCC or Clang compiler
- Make build system
- curl and tar utilities

**Windows:**
- MSYS2/MinGW64 (recommended) or Visual Studio 2022
- curl and tar (included in Windows 10/11)

### Build FFmpeg

**Linux/macOS:**
```bash
cd ext/ext-ffmpeg-core
chmod +x build.sh
./build.sh
```

**Windows:**
```cmd
cd ext\ext-ffmpeg-core
build.bat
```

## Build Output

After successful build, you'll have:

```
ext/ext-ffmpeg-core/ffmpeg-build/
├── bin/                    # FFmpeg binaries (if enabled)
├── include/                # Header files
│   ├── libavcodec/
│   ├── libavutil/
│   └── libswresample/
├── lib/                    # Libraries
│   ├── libavcodec.so/.dll/.dylib
│   ├── libavutil.so/.dll/.dylib
│   ├── libswresample.so/.dll/.dylib
│   └── pkgconfig/          # pkg-config files
└── share/                  # Documentation (if enabled)
```

## ShadPS4 Integration

The main ShadPS4 CMakeLists.txt automatically detects this build:

```cmake
# Set the FFmpeg core build path using ext-ffmpeg-core approach
set(EXT_FFMPEG_CORE_PATH "${CMAKE_SOURCE_DIR}/ext/ext-ffmpeg-core")
set(FFMPEG_PATH "${EXT_FFMPEG_CORE_PATH}/ffmpeg-build")
```

## FFmpeg Configuration

This build is optimized for ShadPS4 with:

### Enabled Features
- **Shared libraries** for dynamic linking
- **AAC decoder** for M4AAC audio streams
- **Audio resampling** for format conversion
- **File protocol** for local file access
- **Essential filters** for audio processing

### Disabled Features
- Static libraries (reduces build size)
- Network protocols (not needed for emulation)
- Video codecs (audio-focused build)
- Documentation and programs (emulator-only use)
- Hardware acceleration (software decoding)

## Patches Applied

### ffmpeg.patch
- **C++14 standard** for CUDA compatibility
- **ShadPS4 AAC extensions** for PlayStation 4 audio formats
- **Build optimizations** for emulator integration

## Troubleshooting

### Build Issues

**"configure: command not found"**
- Ensure you're in the correct directory
- Check that FFmpeg source was extracted properly

**"patch: command not found" (Windows)**
- Install Git for Windows (includes patch utility)
- Or manually apply patches from ffmpeg.patch

**"No suitable compiler found"**
- Linux/macOS: Install GCC or Clang
- Windows: Install MSYS2/MinGW64 or Visual Studio 2022

### Runtime Issues

**"Library not found" errors**
- Ensure LD_LIBRARY_PATH includes ffmpeg-build/lib (Linux/macOS)
- Ensure PATH includes ffmpeg-build/bin (Windows)
- Check that all .so/.dll/.dylib files exist

**CMake can't find FFmpeg**
- Verify PKG_CONFIG_PATH includes ffmpeg-build/lib/pkgconfig
- Check that .pc files exist and have correct paths

## Advanced Configuration

### Custom Build Options

Edit the configure command in build.sh/build.bat to customize:

```bash
./configure \
    --prefix="${SCRIPT_DIR}/${BUILD_DIR}" \
    --enable-shared \
    --disable-static \
    # Add your custom options here
```

### Additional Codecs

To enable more audio codecs, modify the configure options:

```bash
--enable-decoder=opus      # Enable OPUS decoder
--enable-decoder=vorbis    # Enable Vorbis decoder
--enable-parser=opus       # Enable OPUS parser
```

## Version Information

- **FFmpeg Version**: 5.1.6 (stable release)
- **ShadPS4 Compatibility**: Latest (July 2025)
- **Patch Version**: 1.0.0
- **Build System**: CMake + Make/MSYS2

## Contributing

When updating FFmpeg or patches:

1. **Test thoroughly** with ShadPS4 games
2. **Update version numbers** in build scripts
3. **Verify cross-platform** compatibility
4. **Document changes** in this README

## License

This follows FFmpeg's licensing:
- **LGPL 2.1+** for library usage
- **GPL 2.0+** if GPL components are enabled

See FFmpeg documentation for detailed licensing information.

## References

- [FFmpeg Official Site](https://ffmpeg.org/)
- [ShadPS4 Main Repository](https://github.com/shadps4-emu/shadPS4)
- [FFmpeg Build Guide](https://trac.ffmpeg.org/wiki/CompilationGuide)
- [ShadPS4 Build Instructions](https://github.com/shadps4-emu/shadPS4/blob/main/documents/)
