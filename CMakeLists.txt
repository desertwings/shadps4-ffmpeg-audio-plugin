cmake_minimum_required(VERSION 3.21)
project(shadPS4)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the FFmpeg core build path using ext-ffmpeg-core approach (ShadPS4 official pattern)
set(EXT_FFMPEG_CORE_PATH "${CMAKE_SOURCE_DIR}/ext/ext-ffmpeg-core")
set(FFMPEG_PATH "${EXT_FFMPEG_CORE_PATH}/ffmpeg-build")

# Verify ext-ffmpeg-core path exists
if(NOT EXISTS "${EXT_FFMPEG_CORE_PATH}")
    message(WARNING "ext-ffmpeg-core path not found: ${EXT_FFMPEG_CORE_PATH}")
    message(STATUS "Please clone ext-ffmpeg-core repository:")
    message(STATUS "  git clone https://github.com/shadps4-emu/ext-ffmpeg-core.git ext/ext-ffmpeg-core")
endif()

# Verify FFmpeg build exists
if(NOT EXISTS "${FFMPEG_PATH}")
    message(WARNING "FFmpeg build not found: ${FFMPEG_PATH}")
    message(STATUS "Please build FFmpeg using ext-ffmpeg-core:")
    message(STATUS "  cd ext/ext-ffmpeg-core && ./build.sh")
endif()

# Audio decoder sources (.sprx module)
add_library(libSceM4aacDec SHARED
    src/core/libraries/audio/OrbisAudioDecoder.cpp
    src/core/libraries/audio/sce_audiodec.cpp
)

set_target_properties(libSceM4aacDec PROPERTIES
    OUTPUT_NAME "libSceM4aacDec"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
)

# Include FFmpeg headers
target_include_directories(libSceM4aacDec PRIVATE 
    "${FFMPEG_PATH}/include"
    "src/core/libraries/audio"
)

# Link against FFmpeg libraries
if(EXISTS "${FFMPEG_PATH}/lib")
    target_link_libraries(libSceM4aacDec PRIVATE
        "${FFMPEG_PATH}/lib/avcodec.lib"
        "${FFMPEG_PATH}/lib/avutil.lib"
        "${FFMPEG_PATH}/lib/swresample.lib"
    )
else()
    message(WARNING "FFmpeg libraries not found in ${FFMPEG_PATH}/lib")
endif()

# Plugin loader module sources (.sprx module)
add_library(libSceAjm SHARED
    src/core/libraries/ajm/ajm_plugin_loader.cpp
    src/core/libraries/ajm/plugin_m4aac.cpp
)

set_target_properties(libSceAjm PROPERTIES
    OUTPUT_NAME "libSceAjm"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sys_modules
)

# Inherit the same FFmpeg include and linkage for the plugin system
target_include_directories(libSceAjm PRIVATE 
    "${FFMPEG_PATH}/include"
    "src/core/libraries/ajm"
    "src/core/libraries/audio"
)

if(EXISTS "${FFMPEG_PATH}/lib")
    target_link_libraries(libSceAjm PRIVATE
        "${FFMPEG_PATH}/lib/avcodec.lib"
        "${FFMPEG_PATH}/lib/avutil.lib"
        "${FFMPEG_PATH}/lib/swresample.lib"
    )
endif()

# Main emulator executable (placeholder - would contain actual shadPS4 sources)
add_executable(shadps4
    src/main.cpp
    src/sdl_window.cpp
    # Add other source files as needed
)

# Link libraries to the main executable
target_link_libraries(shadps4 PRIVATE
    libSceM4aacDec
    libSceAjm
    # Add other dependencies as needed
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(libSceM4aacDec PRIVATE /W4)
    target_compile_options(libSceAjm PRIVATE /W4)
    target_compile_options(shadps4 PRIVATE /W4)
endif()

# Debug/Release configurations
set_target_properties(libSceM4aacDec libSceAjm shadps4 PROPERTIES
    DEBUG_POSTFIX "_d"
)

# Install targets
install(TARGETS libSceM4aacDec libSceAjm
    RUNTIME DESTINATION sys_modules
    LIBRARY DESTINATION sys_modules
)

install(TARGETS shadps4
    RUNTIME DESTINATION bin
)
