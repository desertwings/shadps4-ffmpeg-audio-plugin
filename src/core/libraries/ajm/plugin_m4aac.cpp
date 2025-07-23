/**
 * @file plugin_m4aac.cpp
 * @brief M4AAC Audio Plugin Implementation for ShadPS4
 * 
 * This file implements the M4AAC audio decoding plugin using the
 * plugin interface defined in plugin_interface.h. It leverages the
 * OrbisAudioDecoder for actual FFmpeg-based decoding.
 */

#include "plugin_interface.h"
#include "../audio/OrbisAudioDecoder.h"
#include <iostream>
#include <memory>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace ShadPS4::Audio {

/**
 * @brief M4AAC Audio Plugin Implementation
 * 
 * This class implements the IAudioPlugin interface specifically for M4AAC codec.
 * It uses the OrbisAudioDecoder internally for FFmpeg-based decoding.
 */
class M4aacAudioPlugin : public IAudioPlugin {
public:
    M4aacAudioPlugin();
    ~M4aacAudioPlugin() override;

    // IAudioPlugin interface implementation
    PluginInfo getPluginInfo() const override;
    bool initialize(const AudioFormat& format) override;
    void shutdown() override;
    DecodeResult decode(const uint8_t* inputData, uint32_t inputSize,
                       void* outputBuffer, uint32_t outputBufferSize,
                       uint32_t* outputSize) override;
    AudioFormat getOutputFormat() const override;
    bool reset() override;
    bool supportsCodec(const std::string& codecType) const override;

private:
    std::unique_ptr<OrbisAudioDecoder> decoder;
    AudioFormat inputFormat;
    AudioFormat outputFormat;
    bool isInitialized;

    void updateOutputFormat();
};

M4aacAudioPlugin::M4aacAudioPlugin() 
    : decoder(nullptr)
    , isInitialized(false) {
    
    // Initialize format structures
    inputFormat = {};
    outputFormat = {};
    
    std::cout << "[M4aacPlugin] Plugin instance created" << std::endl;
}

M4aacAudioPlugin::~M4aacAudioPlugin() {
    shutdown();
    std::cout << "[M4aacPlugin] Plugin instance destroyed" << std::endl;
}

PluginInfo M4aacAudioPlugin::getPluginInfo() const {
    PluginInfo info;
    info.name = "M4AAC Decoder";
    info.version = "1.0.0";
    info.codecType = "M4AAC";
    info.apiVersion = PLUGIN_API_VERSION;
    
    return info;
}

bool M4aacAudioPlugin::initialize(const AudioFormat& format) {
    if (isInitialized) {
        std::cout << "[M4aacPlugin] Already initialized, shutting down first" << std::endl;
        shutdown();
    }

    std::cout << "[M4aacPlugin] Initializing with format - Sample Rate: " << format.sampleRate
              << ", Channels: " << format.channels << ", Bits per Sample: " << format.bitsPerSample << std::endl;

    // Validate input format
    if (format.sampleRate == 0 || format.channels == 0) {
        std::cerr << "[M4aacPlugin] Error: Invalid audio format parameters" << std::endl;
        return false;
    }

    // Store input format
    inputFormat = format;

    // Create decoder instance
    decoder = std::make_unique<OrbisAudioDecoder>();
    if (!decoder) {
        std::cerr << "[M4aacPlugin] Error: Failed to create decoder instance" << std::endl;
        return false;
    }

    // Initialize decoder with M4AAC codec
    if (!decoder->initialize(AV_CODEC_ID_AAC, format.sampleRate, format.channels)) {
        std::cerr << "[M4aacPlugin] Error: Failed to initialize decoder" << std::endl;
        decoder.reset();
        return false;
    }

    // Update output format based on decoder capabilities
    updateOutputFormat();

    isInitialized = true;
    std::cout << "[M4aacPlugin] Successfully initialized M4AAC plugin" << std::endl;
    
    return true;
}

void M4aacAudioPlugin::shutdown() {
    if (isInitialized) {
        decoder.reset();
        isInitialized = false;
        std::cout << "[M4aacPlugin] Plugin shutdown completed" << std::endl;
    }
}

DecodeResult M4aacAudioPlugin::decode(const uint8_t* inputData, uint32_t inputSize,
                                     void* outputBuffer, uint32_t outputBufferSize,
                                     uint32_t* outputSize) {
    if (!isInitialized || !decoder) {
        std::cerr << "[M4aacPlugin] Error: Plugin not initialized" << std::endl;
        return DecodeResult::ErrorNotInitialized;
    }

    if (!inputData || inputSize == 0) {
        std::cerr << "[M4aacPlugin] Error: Invalid input data" << std::endl;
        return DecodeResult::ErrorInvalidInput;
    }

    if (!outputBuffer || outputBufferSize == 0 || !outputSize) {
        std::cerr << "[M4aacPlugin] Error: Invalid output parameters" << std::endl;
        return DecodeResult::ErrorInvalidInput;
    }

    // Perform decoding using OrbisAudioDecoder
    int actualOutputSize = 0;
    int result = decoder->decodePacket(inputData, inputSize,
                                      static_cast<uint8_t*>(outputBuffer),
                                      outputBufferSize, &actualOutputSize);

    // Convert decoder result to plugin result
    if (result == 0) {
        *outputSize = actualOutputSize;
        std::cout << "[M4aacPlugin] Successfully decoded " << inputSize 
                  << " bytes to " << actualOutputSize << " bytes" << std::endl;
        return DecodeResult::Success;
    } else if (result == -2) {
        std::cerr << "[M4aacPlugin] Error: Output buffer too small" << std::endl;
        return DecodeResult::ErrorInsufficientBuffer;
    } else if (result == AVERROR_EOF) {
        std::cout << "[M4aacPlugin] End of stream reached" << std::endl;
        return DecodeResult::ErrorEndOfStream;
    } else {
        std::cerr << "[M4aacPlugin] Error: Codec failure with code " << result << std::endl;
        return DecodeResult::ErrorCodecFailure;
    }
}

AudioFormat M4aacAudioPlugin::getOutputFormat() const {
    return outputFormat;
}

bool M4aacAudioPlugin::reset() {
    if (!isInitialized || !decoder) {
        std::cerr << "[M4aacPlugin] Error: Plugin not initialized" << std::endl;
        return false;
    }

    bool result = decoder->reset();
    if (result) {
        std::cout << "[M4aacPlugin] Plugin reset successfully" << std::endl;
    } else {
        std::cerr << "[M4aacPlugin] Error: Failed to reset plugin" << std::endl;
    }

    return result;
}

bool M4aacAudioPlugin::supportsCodec(const std::string& codecType) const {
    // This plugin supports M4AAC codec
    return (codecType == "M4AAC" || codecType == "AAC" || codecType == "m4aac");
}

void M4aacAudioPlugin::updateOutputFormat() {
    // Set output format based on decoder capabilities
    // For M4AAC, we typically output 16-bit PCM
    outputFormat.sampleRate = inputFormat.sampleRate;
    outputFormat.channels = inputFormat.channels;
    outputFormat.bitsPerSample = 16; // 16-bit PCM output
    outputFormat.frameSize = (outputFormat.bitsPerSample / 8) * outputFormat.channels;

    std::cout << "[M4aacPlugin] Output format - Sample Rate: " << outputFormat.sampleRate
              << ", Channels: " << outputFormat.channels 
              << ", Bits per Sample: " << outputFormat.bitsPerSample
              << ", Frame Size: " << outputFormat.frameSize << std::endl;
}

} // namespace ShadPS4::Audio

// Plugin factory functions for dynamic loading
extern "C" {

/**
 * @brief Create a new M4AAC plugin instance
 * @return Pointer to the created plugin instance
 */
ShadPS4::Audio::IAudioPlugin* createPluginInstance() {
    std::cout << "[M4aacPlugin] Creating new plugin instance" << std::endl;
    return new ShadPS4::Audio::M4aacAudioPlugin();
}

/**
 * @brief Destroy a plugin instance
 * @param plugin Pointer to the plugin instance to destroy
 */
void destroyPluginInstance(ShadPS4::Audio::IAudioPlugin* plugin) {
    if (plugin) {
        std::cout << "[M4aacPlugin] Destroying plugin instance" << std::endl;
        delete plugin;
    }
}

/**
 * @brief Get plugin information without creating an instance
 * @return Plugin information structure
 */
ShadPS4::Audio::PluginInfo getPluginInfo() {
    ShadPS4::Audio::PluginInfo info;
    info.name = "M4AAC Decoder";
    info.version = "1.0.0";
    info.codecType = "M4AAC";
    info.apiVersion = ShadPS4::Audio::PLUGIN_API_VERSION;
    
    return info;
}

/**
 * @brief Check if this plugin supports a specific codec
 * @param codecType String identifier for the codec
 * @return true if supported, false otherwise
 */
bool supportsCodec(const char* codecType) {
    if (!codecType) return false;
    
    std::string codec(codecType);
    return (codec == "M4AAC" || codec == "AAC" || codec == "m4aac");
}

} // extern "C"
