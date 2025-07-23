# 📁 How to Copy the ShadPS4 FFmpeg Project to D:\moddedchadps4\shadPS4\

## 🎯 Overview

This guide explains how to copy the complete ShadPS4 FFmpeg audio integration project from your current workspace to the target directory `D:\moddedchadps4\shadPS4\`.

## 📋 Current Project Structure

Your workspace contains the complete implementation:

```
/project/sandbox/user-workspace/
├── CMakeLists.txt                          # Main build configuration
├── README.md                               # Project documentation
├── IMPLEMENTATION_SUMMARY.md               # Technical details
├── DOWNLOAD_SETUP_GUIDE.md                 # Setup instructions
├── build.bat                               # Windows build script
├── externals/
│   └── ffmpeg-core/                        # FFmpeg integration point
├── src/
│   ├── main.cpp                            # Main application
│   ├── sdl_window.cpp                      # SDL window placeholder
│   └── core/
│       └── libraries/
│           ├── audio/                      # Core audio decoding
│           │   ├── OrbisAudioDecoder.h
│           │   ├── OrbisAudioDecoder.cpp
│           │   └── sce_audiodec.cpp
│           └── ajm/                        # Plugin system
│               ├── plugin_interface.h
│               ├── plugin_m4aac.cpp
│               └── ajm_plugin_loader.cpp
└── sys_modules/                            # Output directory
```

## 🔄 Copy Methods

### Method 1: Windows File Explorer (Easiest)

1. **Open File Explorer**
2. **Navigate to your current workspace location**
3. **Select all project files and folders**:
   - `CMakeLists.txt`
   - `README.md`
   - `IMPLEMENTATION_SUMMARY.md`
   - `DOWNLOAD_SETUP_GUIDE.md`
   - `build.bat`
   - `externals/` folder
   - `src/` folder
   - `sys_modules/` folder

4. **Copy files** (Ctrl+C)
5. **Create target directory**:
   ```
   D:\moddedchadps4\shadPS4\
   ```
6. **Paste files** (Ctrl+V) into the target directory

### Method 2: Command Line (Windows)

```cmd
REM Create target directory
mkdir "D:\moddedchadps4\shadPS4"

REM Copy all files and folders
xcopy /E /I /H /Y "C:\path\to\your\workspace\*" "D:\moddedchadps4\shadPS4\"
```

**Replace** `C:\path\to\your\workspace\` with your actual workspace path.

### Method 3: PowerShell (Windows)

```powershell
# Create target directory
New-Item -ItemType Directory -Force -Path "D:\moddedchadps4\shadPS4"

# Copy all files recursively
Copy-Item -Path "C:\path\to\your\workspace\*" -Destination "D:\moddedchadps4\shadPS4\" -Recurse -Force
```

### Method 4: Git Clone (If using version control)

If you've committed the project to a Git repository:

```bash
cd D:\moddedchadps4
git clone https://github.com/your-username/shadPS4-ffmpeg-audio.git shadPS4
```

## 📂 Step-by-Step Copy Process

### Step 1: Prepare Target Directory
```cmd
# Create the main directory
mkdir "D:\moddedchadps4"
mkdir "D:\moddedchadps4\shadPS4"
```

### Step 2: Copy Root Files
Copy these files to `D:\moddedchadps4\shadPS4\`:
- ✅ `CMakeLists.txt`
- ✅ `README.md`
- ✅ `IMPLEMENTATION_SUMMARY.md`
- ✅ `DOWNLOAD_SETUP_GUIDE.md`
- ✅ `build.bat`

### Step 3: Copy Directory Structure
Copy these folders with all contents:
- ✅ `externals/` → `D:\moddedchadps4\shadPS4\externals\`
- ✅ `src/` → `D:\moddedchadps4\shadPS4\src\`
- ✅ `sys_modules/` → `D:\moddedchadps4\shadPS4\sys_modules\`

### Step 4: Verify Copy Success
After copying, your target directory should look like:

```
D:\moddedchadps4\shadPS4\
├── CMakeLists.txt                          ✅
├── README.md                               ✅
├── IMPLEMENTATION_SUMMARY.md               ✅
├── DOWNLOAD_SETUP_GUIDE.md                 ✅
├── build.bat                               ✅
├── externals\
│   └── ffmpeg-core\                        ✅
├── src\
│   ├── main.cpp                            ✅
│   ├── sdl_window.cpp                      ✅
│   └── core\
│       └── libraries\
│           ├── audio\
│           │   ├── OrbisAudioDecoder.h     ✅
│           │   ├── OrbisAudioDecoder.cpp   ✅
│           │   └── sce_audiodec.cpp        ✅
│           └── ajm\
│               ├── plugin_interface.h      ✅
│               ├── plugin_m4aac.cpp        ✅
│               └── ajm_plugin_loader.cpp   ✅
└── sys_modules\                            ✅
```

## 🔧 Post-Copy Setup

### 1. Download FFmpeg Libraries
After copying the project, you need to download FFmpeg:

```cmd
cd "D:\moddedchadps4\shadPS4"
```

1. **Download FFmpeg** from: https://github.com/BtbN/FFmpeg-Builds/releases
2. **Choose**: `ffmpeg-master-latest-win64-gpl-shared.zip`
3. **Extract to**: `D:\moddedchadps4\shadPS4\externals\ffmpeg-core\`
4. **Rename folder** to: `ffmpeg-2024-01-15-git-full_build` (or update CMakeLists.txt)

### 2. Verify FFmpeg Structure
Ensure this structure exists:
```
D:\moddedchadps4\shadPS4\externals\ffmpeg-core\ffmpeg-2024-01-15-git-full_build\
├── bin\
├── include\
│   ├── libavcodec\
│   ├── libavutil\
│   └── libswresample\
└── lib\
    ├── avcodec.lib
    ├── avutil.lib
    └── swresample.lib
```

### 3. Build the Project
```cmd
cd "D:\moddedchadps4\shadPS4"
build.bat
```

## ✅ Verification Checklist

After copying and setup, verify:

- ✅ **All files copied**: Check that all source files are present
- ✅ **Directory structure intact**: Verify folder hierarchy is correct
- ✅ **FFmpeg installed**: Ensure FFmpeg libraries are in place
- ✅ **Build successful**: Run `build.bat` and check for errors
- ✅ **Output files created**: Verify `.dll` files in `sys_modules\`

## 🚨 Common Issues & Solutions

### Issue: "Access Denied" during copy
**Solution**: Run command prompt as Administrator

### Issue: "Path too long" error
**Solution**: Use shorter intermediate paths or enable long path support in Windows

### Issue: Files missing after copy
**Solution**: Ensure you're copying from the correct source directory and include hidden files

### Issue: Build fails after copy
**Solution**: 
1. Verify FFmpeg is properly installed
2. Check that all source files were copied
3. Ensure Visual Studio 2022 is installed

## 🎉 Success!

Once copied and built successfully, you'll have:
- ✅ Complete ShadPS4 FFmpeg audio integration
- ✅ M4AAC decoding capabilities
- ✅ Plugin system for additional codecs
- ✅ Ready for Dreams (CUSA04301) testing

Your ShadPS4 fork with FFmpeg audio integration is now ready at `D:\moddedchadps4\shadPS4\`!
