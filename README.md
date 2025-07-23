# ShadPS4 FFmpeg Audio Integration & Plugin System

This implementation provides FFmpeg-powered audio decoding capabilities and a flexible plugin system for the ShadPS4 PlayStation 4 emulator.

## 🎯 Overview

This project implements:
- **FFmpeg-based M4AAC audio decoding** using `avcodec_send_packet()`, `avcodec_receive_frame()`, and `swr_convert()`
- **Plugin system architecture** for supporting multiple audio codecs
- **SCE audio decoder interface** compatible with PS4 audio subsystem
- **Dynamic plugin loading** for future codec expansion

## 📁 Project Structure

```
D:\moddedchadps4\shadPS4\
├── CMakeLists.txt                          # Main build configuration
├── README.md                               # This documentation
├── ext/                                    # External dependencies (ShadPS4 pattern)
│   └── ext-ffmpeg-core/                    # FFmpeg integration module
│       ├── README.md                       # FFmpeg build documentation
│       ├── ffmpeg.patch                    # ShadPS4-specific patches
│       ├── build.sh                        # Linux/macOS build script
│       ├── build.bat                       # Windows build script
│       └── ffmpeg-build/                   # Built FFmpeg libraries (after build)
│           ├── include/                    # FFmpeg headers
│           └── lib/                        # FFmpeg libraries
├── src/
│   ├── main.cpp                            # Main application entry point
│   ├── sdl_window.cpp                      # SDL window placeholder
│   └── core/
│       └── libraries/
│           ├── audio/                      # Core audio decoding
│           │   ├── OrbisAudioDecoder.h     # FFmpeg decoder header
│           │   ├── OrbisAudioDecoder.cpp   # FFmpeg decoder implementation
│           │   └── sce_audiodec.cpp        # SCE audio interface
│           └── ajm/                        # Plugin system
│               ├── plugin_interface.h      # Plugin ABI definition
│               ├── plugin_m4aac.cpp        # M4AAC plugin implementation
│               └── ajm_plugin_loader.cpp   # Plugin loader system
└── sys_modules/                            # Output directory for .sprx modules
    ├── libSceM4aacDec.sprx                 # Audio decoder module
    └── libSceAjm.sprx                      # Plugin loader module
```

## 🔧 Prerequisites

### Required Software
- **Visual Studio 2022** with C++ Desktop Development workload
- **CMake 3.21+**
- **Git**
- **FFmpeg** (manually built for Windows with MSVC)

### FFmpeg Setup (ext-ffmpeg-core)
ShadPS4 uses the official `ext-ffmpeg-core` approach for FFmpeg integration:

1. **Clone ext-ffmpeg-core** (if not already included):
   ```bash
   git clone https://github.com/shadps4-emu/ext-ffmpeg-core.git ext/ext-ffmpeg-core
   ```

2. **Build FFmpeg** using the provided scripts:
   ```bash
   # Linux/macOS
   cd ext/ext-ffmpeg-core
   chmod +x build.sh
   ./build.sh
   
   # Windows
   cd ext\ext-ffmpeg-core
   build.bat
   ```

3. **Verify build structure**:
   ```
   ext/ext-ffmpeg-core/ffmpeg-build/
   ├── include/
   │   ├── libavcodec/
   │   ├── libavutil/
   │   └── libswresample/
   └── lib/
       ├── libavcodec.so/.dll/.dylib
       ├── libavutil.so/.dll/.dylib
       └── libswresample.so/.dll/.dylib
   ```

## 🏗️ Building

### Windows (Visual Studio)
```bash
# Clone the repository
cd D:\moddedchadps4
git clone https://github.com/georgemoralis/shadPS4.git
cd shadPS4

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64

# Build the project
cmake --build . --config Release

# Or open in Visual Studio
start shadPS4.sln
```

### Command Line Build
```bash
# Configure and build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 🧩 Architecture

### Core Components

#### 1. OrbisAudioDecoder (`src/core/libraries/audio/`)
- **Purpose**: FFmpeg-based audio decoding engine
- **Key Features**:
  - M4AAC to PCM conversion
  - Sample rate and channel format handling
  - Error handling and logging
  - Resource management

#### 2. SCE Audio Interface (`src/core/libraries/audio/sce_audiodec.cpp`)
- **Purpose**: PlayStation 4 compatible audio decoder interface
- **Key Features**:
  - Instance management
  - Thread-safe operations
  - Error code translation
  - Memory management

#### 3. Plugin System (`src/core/libraries/ajm/`)
- **Purpose**: Extensible codec plugin architecture
- **Key Features**:
  - Abstract plugin interface (`IAudioPlugin`)
  - Dynamic plugin loading
  - Built-in plugin registration
  - Plugin lifecycle management

### Plugin Interface

```cpp
class IAudioPlugin {
public:
    virtual PluginInfo getPluginInfo() const = 0;
    virtual bool initialize(const AudioFormat& format) = 0;
    virtual DecodeResult decode(const uint8_t* inputData, uint32_t inputSize,
                               void* outputBuffer, uint32_t outputBufferSize,
                               uint32_t* outputSize) = 0;
    virtual AudioFormat getOutputFormat() const = 0;
    virtual bool reset() = 0;
    virtual bool supportsCodec(const std::string& codecType) const = 0;
};
```

## 🧪 Testing

### Basic Functionality Test
```bash
# Run the built executable
./build/shadps4

# Expected output:
# [AjmPluginLoader] Initializing plugin system
# [M4aacPlugin] Plugin instance created
# [AjmPluginLoader] Plugin M4aacDec registered
# [sceAjm] AJM instance created successfully
```

### Game Testing
1. Launch **Dreams (CUSA04301)** in the emulator
2. Monitor console output for:
   ```
   [Ajm] Plugin M4aacDec registered
   Decoded frame: sample_rate=48000, channels=2
   ```
3. Verify no crash occurs at `sceAjmInstanceCreate()`

## 🔍 API Reference

### SCE Audio Decoder Functions

```c
// Create decoder instance
int sceAudioDecCreateDecoder(const SceAudioDecConfig* config, 
                            SceAudioDecInstance** instance);

// Decode audio packet
int sceAudioDecDecode(SceAudioDecInstance* instance,
                     const void* inputData, uint32_t inputSize,
                     void* outputData, uint32_t* outputSize);

// Reset decoder state
int sceAudioDecReset(SceAudioDecInstance* instance);

// Destroy decoder instance
int sceAudioDecDeleteDecoder(SceAudioDecInstance* instance);
```

### AJM Plugin System Functions

```c
// Initialize plugin system
int sceAjmInstanceCreate();

// Get plugin for codec
IAudioPlugin* sceAjmGetPlugin(const char* codecType);

// Shutdown plugin system
int sceAjmInstanceDestroy();
```

## 🚀 Future Enhancements

### Planned Features
1. **Additional Codec Support**
   - AT9 audio codec
   - OPUS codec
   - PCM variants

2. **Dynamic Plugin Loading**
   ```cpp
   // Load external plugin DLL
   bool loadDynamicPlugin(const std::string& pluginPath);
   
   // Example usage
   loader.loadDynamicPlugin("plugins/plugin_opus.dll");
   ```

3. **Performance Optimizations**
   - Multi-threaded decoding
   - Buffer pooling
   - SIMD optimizations

4. **Advanced Features**
   - Real-time format conversion
   - Audio effects processing
   - Surround sound support

## 🐛 Troubleshooting

### Common Issues

#### FFmpeg Not Found
```
Error: FFmpeg path not found: externals/ffmpeg-core/ffmpeg-YYYY-MM-DD-<hash>-full_build
```
**Solution**: Verify FFmpeg is properly installed in the externals directory.

#### Plugin Registration Failed
```
Error: Failed to create M4AAC plugin
```
**Solution**: Check FFmpeg libraries are properly linked and accessible.

#### Decoder Initialization Failed
```
Error: Failed to initialize decoder
```
**Solution**: Verify audio format parameters and FFmpeg codec availability.

### Debug Build
For detailed debugging, build with debug configuration:
```bash
cmake --build build --config Debug
```

## 📄 License

This project follows the same license as the base ShadPS4 project (GPL-2.0).

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Implement changes with proper error handling
4. Add tests for new functionality
5. Submit a pull request

## 📚 References

- [FFmpeg Documentation](https://ffmpeg.org/documentation.html)
- [ShadPS4 Base Repository](https://github.com/georgemoralis/shadPS4)
- [PS4 Audio System Documentation](https://github.com/idc/ps4libdoc)
- [FFmpeg Builds](https://github.com/BtbN/FFmpeg-Builds)

---

**Note**: This implementation provides the foundation for FFmpeg-based audio decoding in ShadPS4. For production use, additional testing and optimization may be required.
