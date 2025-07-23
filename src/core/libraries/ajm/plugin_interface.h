#pragma once

#include <cstdint>
#include <string>

/**
 * @file plugin_interface.h
 * @brief Audio Plugin Interface for ShadPS4 AJM System
 * 
 * This header defines the Application Binary Interface (ABI) for audio decoding plugins.
 * All audio codec plugins must implement this interface to be compatible with the
 * ShadPS4 plugin loader system.
 */

namespace ShadPS4::Audio {

/**
 * @brief Plugin information structure
 */
struct PluginInfo {
    std::string name;           // Plugin name (e.g., "M4AAC Decoder")
    std::string version;        // Plugin version (e.g., "1.0.0")
    std::string codecType;      // Codec type (e.g., "M4AAC", "OPUS", "AT9")
    uint32_t apiVersion;        // API version for compatibility checking
};

/**
 * @brief Audio format information
 */
struct AudioFormat {
    uint32_t sampleRate;        // Sample rate in Hz (e.g., 48000)
    uint16_t channels;          // Number of channels (1=mono, 2=stereo, etc.)
    uint16_t bitsPerSample;     // Bits per sample (typically 16 or 24)
    uint32_t frameSize;         // Size of one audio frame in bytes
};

/**
 * @brief Decoding result codes
 */
enum class DecodeResult : int32_t {
    Success = 0,                // Decoding successful
    ErrorInvalidInput = -1,     // Invalid input data
    ErrorInsufficientBuffer = -2, // Output buffer too small
    ErrorCodecFailure = -3,     // Codec-specific error
    ErrorNotInitialized = -4,   // Plugin not properly initialized
    ErrorEndOfStream = -5       // End of audio stream reached
};

/**
 * @brief Abstract base class for audio plugins
 * 
 * All audio codec plugins must inherit from this class and implement
 * all pure virtual methods.
 */
class IAudioPlugin {
public:
    virtual ~IAudioPlugin() = default;

    /**
     * @brief Get plugin information
     * @return PluginInfo structure containing plugin details
     */
    virtual PluginInfo getPluginInfo() const = 0;

    /**
     * @brief Initialize the plugin with specific audio format
     * @param format Input audio format parameters
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize(const AudioFormat& format) = 0;

    /**
     * @brief Shutdown and cleanup the plugin
     */
    virtual void shutdown() = 0;

    /**
     * @brief Decode an audio packet
     * @param inputData Pointer to compressed audio data
     * @param inputSize Size of input data in bytes
     * @param outputBuffer Pointer to output PCM buffer
     * @param outputBufferSize Size of output buffer in bytes
     * @param outputSize Pointer to store actual output size
     * @return DecodeResult indicating success or failure
     */
    virtual DecodeResult decode(
        const uint8_t* inputData,
        uint32_t inputSize,
        void* outputBuffer,
        uint32_t outputBufferSize,
        uint32_t* outputSize
    ) = 0;

    /**
     * @brief Get the output format after decoding
     * @return AudioFormat structure with output format details
     */
    virtual AudioFormat getOutputFormat() const = 0;

    /**
     * @brief Reset the decoder state
     * @return true if reset successful, false otherwise
     */
    virtual bool reset() = 0;

    /**
     * @brief Check if the plugin can handle the specified codec type
     * @param codecType String identifier for the codec (e.g., "M4AAC")
     * @return true if plugin supports this codec, false otherwise
     */
    virtual bool supportsCodec(const std::string& codecType) const = 0;
};

/**
 * @brief Plugin factory function type
 * 
 * Each plugin DLL should export a function of this type named "createPluginInstance"
 * to allow dynamic loading of plugins.
 */
typedef IAudioPlugin* (*CreatePluginInstanceFunc)();

/**
 * @brief Plugin destruction function type
 * 
 * Each plugin DLL should export a function of this type named "destroyPluginInstance"
 * to properly clean up plugin instances.
 */
typedef void (*DestroyPluginInstanceFunc)(IAudioPlugin* plugin);

// API version constant for compatibility checking
constexpr uint32_t PLUGIN_API_VERSION = 0x00010000; // Version 1.0.0

} // namespace ShadPS4::Audio
