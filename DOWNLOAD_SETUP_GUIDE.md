# 📥 ShadPS4 FFmpeg Audio Integration - Download & Setup Guide

## 🎯 Quick Start

This guide will help you download, set up, and build the ShadPS4 FFmpeg audio integration system.

## 📋 Method 1: Download Project Files

### Option A: Download as ZIP (Easiest)
1. **Download the project files** from your current workspace
2. **Extract to your desired location**:
   ```
   D:\moddedchadps4\shadPS4\
   ```

### Option B: Clone from Repository
If this project is hosted on GitHub:
```bash
cd D:\moddedchadps4
git clone https://github.com/your-username/shadPS4-ffmpeg-audio.git shadPS4
cd shadPS4
```

## 🔧 Prerequisites Download & Setup

### 1. Visual Studio 2022
**Download**: https://visualstudio.microsoft.com/vs/
- Install with **C++ Desktop Development** workload
- Ensure CMake tools are included

### 2. CMake (if not included with VS)
**Download**: https://cmake.org/download/
- Version 3.21 or later required
- Add to system PATH during installation

### 3. Git
**Download**: https://git-scm.com/download/win
- Required for version control and some build processes

### 4. FFmpeg Libraries (CRITICAL)
**Option A: Pre-built Binaries (Recommended)**
1. **Download from**: https://github.com/BtbN/FFmpeg-Builds/releases
2. **Choose**: `ffmpeg-master-latest-win64-gpl-shared.zip`
3. **Extract to**: `D:\moddedchadps4\shadPS4\externals\ffmpeg-core\`
4. **Rename folder** to match the pattern: `ffmpeg-YYYY-MM-DD-<hash>-full_build`

**Option B: Build FFmpeg Yourself**
```bash
# This is advanced - only if you need custom FFmpeg build
git clone https://git.ffmpeg.org/ffmpeg.git
# Follow FFmpeg Windows build instructions
```

## 📁 Final Directory Structure

After downloading and setting up, your directory should look like:

```
D:\moddedchadps4\shadPS4\
├── CMakeLists.txt
├── README.md
├── IMPLEMENTATION_SUMMARY.md
├── DOWNLOAD_SETUP_GUIDE.md
├── build.bat
├── externals/
│   └── ffmpeg-core/
│       └── ffmpeg-2024-01-15-git-full_build/    # ← FFmpeg goes here
│           ├── bin/
│           ├── include/
│           │   ├── libavcodec/
│           │   ├── libavutil/
│           │   └── libswresample/
│           └── lib/
│               ├── avcodec.lib
│               ├── avutil.lib
│               └── swresample.lib
├── src/
│   ├── main.cpp
│   ├── sdl_window.cpp
│   └── core/
│       └── libraries/
│           ├── audio/
│           │   ├── OrbisAudioDecoder.h
│           │   ├── OrbisAudioDecoder.cpp
│           │   └── sce_audiodec.cpp
│           └── ajm/
│               ├── plugin_interface.h
│               ├── plugin_m4aac.cpp
│               └── ajm_plugin_loader.cpp
└── sys_modules/                                 # ← Build output goes here
```

## 🏗️ Build Instructions

### Method 1: Automated Build (Recommended)
```bash
# Navigate to project directory
cd D:\moddedchadps4\shadPS4

# Run the build script
build.bat
```

### Method 2: Manual Build
```bash
# Navigate to project directory
cd D:\moddedchadps4\shadPS4

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build the project
cmake --build . --config Release
```

### Method 3: Visual Studio IDE
```bash
# Generate Visual Studio solution
cd D:\moddedchadps4\shadPS4
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# Open in Visual Studio
start shadPS4.sln
```

## ✅ Verify Installation

### 1. Check Build Output
After successful build, verify these files exist:
```
build/
├── sys_modules/
│   ├── libSceM4aacDec.dll    # ✅ Audio decoder module
│   └── libSceAjm.dll         # ✅ Plugin loader module
└── Release/
    └── shadps4.exe           # ✅ Main executable
```

### 2. Test the System
```bash
# Run the test application
cd D:\moddedchadps4\shadPS4\build
Release\shadps4.exe
```

**Expected Output:**
```
ShadPS4 Emulator - FFmpeg Audio Integration Demo
=================================================
[AjmPluginLoader] Initializing plugin system
[M4aacPlugin] Plugin instance created
[AjmPluginLoader] Plugin M4aacDec registered
[sceAjm] AJM instance created successfully
M4AAC plugin found and ready!
✓ FFmpeg-based M4AAC audio decoding
✓ Plugin system for audio codecs
✓ SCE audio decoder interface
✓ Dynamic plugin loading support
```

## 🔧 Troubleshooting

### Common Issues & Solutions

#### ❌ "FFmpeg path not found"
**Problem**: FFmpeg not properly installed
**Solution**: 
1. Download FFmpeg from https://github.com/BtbN/FFmpeg-Builds/releases
2. Extract to `externals/ffmpeg-core/`
3. Ensure folder name matches pattern in CMakeLists.txt

#### ❌ "CMake not found"
**Problem**: CMake not installed or not in PATH
**Solution**: 
1. Download CMake from https://cmake.org/download/
2. Install with "Add to PATH" option checked
3. Restart command prompt

#### ❌ "Visual Studio not found"
**Problem**: Visual Studio 2022 not installed
**Solution**: 
1. Download from https://visualstudio.microsoft.com/vs/
2. Install with C++ Desktop Development workload

#### ❌ Build errors with FFmpeg
**Problem**: Wrong FFmpeg version or architecture
**Solution**: 
1. Use 64-bit FFmpeg build
2. Ensure static libraries (.lib files) are present
3. Check include directories exist

## 🎮 Integration with ShadPS4

### For Existing ShadPS4 Users
1. **Backup your current ShadPS4 installation**
2. **Replace/merge** the audio system files:
   - Copy `src/core/libraries/audio/` to your ShadPS4 source
   - Copy `src/core/libraries/ajm/` to your ShadPS4 source
   - Update your CMakeLists.txt with the FFmpeg integration
3. **Rebuild** your ShadPS4 with the new audio system

### For New Users
1. **Use this as your base ShadPS4 fork**
2. **Add additional ShadPS4 components** as needed
3. **Integrate with the main ShadPS4 codebase** following their contribution guidelines

## 📚 Additional Resources

- **ShadPS4 Main Repository**: https://github.com/shadps4-emu/shadPS4
- **George Moralis Fork**: https://github.com/georgemoralis/shadPS4
- **FFmpeg Documentation**: https://ffmpeg.org/documentation.html
- **FFmpeg Windows Builds**: https://github.com/BtbN/FFmpeg-Builds
- **CMake Documentation**: https://cmake.org/documentation/

## 🆘 Getting Help

If you encounter issues:

1. **Check the logs** - Build output contains detailed error information
2. **Verify prerequisites** - Ensure all required software is installed
3. **Check file paths** - Ensure FFmpeg is in the correct location
4. **Review documentation** - README.md contains additional troubleshooting info

## 🎉 Success!

Once you see the test output showing successful plugin registration, your ShadPS4 FFmpeg audio integration is ready to use!

You can now:
- ✅ Decode M4AAC audio streams
- ✅ Use the plugin system for additional codecs
- ✅ Integrate with games like Dreams (CUSA04301)
- ✅ Extend with additional audio format support
