/**
 * @file sce_audiodec.cpp
 * @brief SCE Audio Decoder implementation for ShadPS4
 * 
 * This file implements the SCE audio decoder interface that bridges
 * the emulator's audio system with the FFmpeg-based decoder.
 */

#include "OrbisAudioDecoder.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>

extern "C" {
    #include <libavcodec/avcodec.h>
}

namespace ShadPS4::Audio {

// Global decoder instance management
static std::unordered_map<int, std::unique_ptr<OrbisAudioDecoder>> g_decoders;
static std::mutex g_decoderMutex;
static int g_nextDecoderId = 1;

/**
 * @brief SCE Audio Decoder error codes
 */
enum SceAudioDecError {
    SCE_AUDIODEC_OK = 0,
    SCE_AUDIODEC_ERROR_INVALID_PARAM = -1,
    SCE_AUDIODEC_ERROR_INVALID_STATE = -2,
    SCE_AUDIODEC_ERROR_INSUFFICIENT_BUFFER = -3,
    SCE_AUDIODEC_ERROR_CODEC_NOT_SUPPORTED = -4,
    SCE_AUDIODEC_ERROR_DECODE_FAILED = -5
};

/**
 * @brief Audio codec types supported by SCE
 */
enum SceAudioCodecType {
    SCE_AUDIODEC_TYPE_M4AAC = 0x2001,
    SCE_AUDIODEC_TYPE_AT9 = 0x2002,
    SCE_AUDIODEC_TYPE_OPUS = 0x2003
};

/**
 * @brief Audio decoder configuration structure
 */
struct SceAudioDecConfig {
    uint32_t codecType;        // Codec type (SceAudioCodecType)
    uint32_t sampleRate;       // Sample rate in Hz
    uint16_t channels;         // Number of channels
    uint16_t reserved;         // Reserved for alignment
};

/**
 * @brief Audio decoder instance structure
 */
struct SceAudioDecInstance {
    int decoderId;             // Internal decoder ID
    SceAudioDecConfig config;  // Decoder configuration
    bool isInitialized;        // Initialization state
};

} // namespace ShadPS4::Audio

using namespace ShadPS4::Audio;

extern "C" {

/**
 * @brief Create an audio decoder instance
 * @param config Pointer to decoder configuration
 * @param instance Pointer to store the created instance
 * @return SCE_AUDIODEC_OK on success, error code on failure
 */
int sceAudioDecCreateDecoder(const SceAudioDecConfig* config, SceAudioDecInstance** instance) {
    if (!config || !instance) {
        std::cerr << "[sceAudioDec] Error: Invalid parameters for CreateDecoder" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_PARAM;
    }

    std::cout << "[sceAudioDec] Creating decoder - Type: 0x" << std::hex << config->codecType 
              << ", Sample Rate: " << std::dec << config->sampleRate 
              << ", Channels: " << config->channels << std::endl;

    // Validate codec type
    AVCodecID codecId;
    switch (config->codecType) {
        case SCE_AUDIODEC_TYPE_M4AAC:
            codecId = AV_CODEC_ID_AAC;
            break;
        case SCE_AUDIODEC_TYPE_AT9:
            std::cerr << "[sceAudioDec] Error: AT9 codec not yet supported" << std::endl;
            return SCE_AUDIODEC_ERROR_CODEC_NOT_SUPPORTED;
        case SCE_AUDIODEC_TYPE_OPUS:
            std::cerr << "[sceAudioDec] Error: OPUS codec not yet supported" << std::endl;
            return SCE_AUDIODEC_ERROR_CODEC_NOT_SUPPORTED;
        default:
            std::cerr << "[sceAudioDec] Error: Unsupported codec type: 0x" 
                      << std::hex << config->codecType << std::endl;
            return SCE_AUDIODEC_ERROR_CODEC_NOT_SUPPORTED;
    }

    // Create decoder instance
    auto decoder = std::make_unique<OrbisAudioDecoder>();
    if (!decoder->initialize(codecId, config->sampleRate, config->channels)) {
        std::cerr << "[sceAudioDec] Error: Failed to initialize decoder" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    // Create SCE instance structure
    auto sceInstance = new SceAudioDecInstance();
    sceInstance->config = *config;
    sceInstance->isInitialized = true;

    // Store decoder with unique ID
    std::lock_guard<std::mutex> lock(g_decoderMutex);
    int decoderId = g_nextDecoderId++;
    sceInstance->decoderId = decoderId;
    g_decoders[decoderId] = std::move(decoder);

    *instance = sceInstance;

    std::cout << "[sceAudioDec] Successfully created decoder with ID: " << decoderId << std::endl;
    return SCE_AUDIODEC_OK;
}

/**
 * @brief Delete an audio decoder instance
 * @param instance Pointer to the decoder instance
 * @return SCE_AUDIODEC_OK on success, error code on failure
 */
int sceAudioDecDeleteDecoder(SceAudioDecInstance* instance) {
    if (!instance) {
        std::cerr << "[sceAudioDec] Error: Invalid instance for DeleteDecoder" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_PARAM;
    }

    std::cout << "[sceAudioDec] Deleting decoder with ID: " << instance->decoderId << std::endl;

    // Remove decoder from global map
    {
        std::lock_guard<std::mutex> lock(g_decoderMutex);
        auto it = g_decoders.find(instance->decoderId);
        if (it != g_decoders.end()) {
            g_decoders.erase(it);
        }
    }

    // Clean up instance
    delete instance;

    std::cout << "[sceAudioDec] Successfully deleted decoder" << std::endl;
    return SCE_AUDIODEC_OK;
}

/**
 * @brief Decode an audio packet
 * @param instance Pointer to the decoder instance
 * @param inputData Pointer to compressed audio data
 * @param inputSize Size of input data in bytes
 * @param outputData Pointer to output PCM buffer
 * @param outputSize Pointer to output buffer size (in/out parameter)
 * @return SCE_AUDIODEC_OK on success, error code on failure
 */
int sceAudioDecDecode(SceAudioDecInstance* instance, 
                     const void* inputData, uint32_t inputSize,
                     void* outputData, uint32_t* outputSize) {
    if (!instance || !inputData || inputSize == 0 || !outputData || !outputSize) {
        std::cerr << "[sceAudioDec] Error: Invalid parameters for Decode" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_PARAM;
    }

    if (!instance->isInitialized) {
        std::cerr << "[sceAudioDec] Error: Decoder not initialized" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    // Get decoder instance
    std::unique_ptr<OrbisAudioDecoder>* decoder = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_decoderMutex);
        auto it = g_decoders.find(instance->decoderId);
        if (it == g_decoders.end()) {
            std::cerr << "[sceAudioDec] Error: Decoder not found for ID: " 
                      << instance->decoderId << std::endl;
            return SCE_AUDIODEC_ERROR_INVALID_STATE;
        }
        decoder = &(it->second);
    }

    // Perform decoding
    int actualOutputSize = 0;
    int result = (*decoder)->decodePacket(
        static_cast<const uint8_t*>(inputData), inputSize,
        static_cast<uint8_t*>(outputData), *outputSize, &actualOutputSize
    );

    if (result < 0) {
        std::cerr << "[sceAudioDec] Error: Decode failed with code: " << result << std::endl;
        return SCE_AUDIODEC_ERROR_DECODE_FAILED;
    }

    *outputSize = actualOutputSize;

    std::cout << "[sceAudioDec] Successfully decoded " << inputSize 
              << " bytes to " << actualOutputSize << " bytes" << std::endl;

    return SCE_AUDIODEC_OK;
}

/**
 * @brief Reset the decoder state
 * @param instance Pointer to the decoder instance
 * @return SCE_AUDIODEC_OK on success, error code on failure
 */
int sceAudioDecReset(SceAudioDecInstance* instance) {
    if (!instance) {
        std::cerr << "[sceAudioDec] Error: Invalid instance for Reset" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_PARAM;
    }

    if (!instance->isInitialized) {
        std::cerr << "[sceAudioDec] Error: Decoder not initialized" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    // Get decoder instance
    std::unique_ptr<OrbisAudioDecoder>* decoder = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_decoderMutex);
        auto it = g_decoders.find(instance->decoderId);
        if (it == g_decoders.end()) {
            std::cerr << "[sceAudioDec] Error: Decoder not found for ID: " 
                      << instance->decoderId << std::endl;
            return SCE_AUDIODEC_ERROR_INVALID_STATE;
        }
        decoder = &(it->second);
    }

    // Reset decoder
    if (!(*decoder)->reset()) {
        std::cerr << "[sceAudioDec] Error: Failed to reset decoder" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    std::cout << "[sceAudioDec] Successfully reset decoder" << std::endl;
    return SCE_AUDIODEC_OK;
}

/**
 * @brief Get decoder information
 * @param instance Pointer to the decoder instance
 * @param info Pointer to store decoder information
 * @return SCE_AUDIODEC_OK on success, error code on failure
 */
int sceAudioDecGetInfo(SceAudioDecInstance* instance, DecoderInfo* info) {
    if (!instance || !info) {
        std::cerr << "[sceAudioDec] Error: Invalid parameters for GetInfo" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_PARAM;
    }

    if (!instance->isInitialized) {
        std::cerr << "[sceAudioDec] Error: Decoder not initialized" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    // Get decoder instance
    std::unique_ptr<OrbisAudioDecoder>* decoder = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_decoderMutex);
        auto it = g_decoders.find(instance->decoderId);
        if (it == g_decoders.end()) {
            std::cerr << "[sceAudioDec] Error: Decoder not found for ID: " 
                      << instance->decoderId << std::endl;
            return SCE_AUDIODEC_ERROR_INVALID_STATE;
        }
        decoder = &(it->second);
    }

    // Get decoder information
    if (!(*decoder)->getDecoderInfo(*info)) {
        std::cerr << "[sceAudioDec] Error: Failed to get decoder info" << std::endl;
        return SCE_AUDIODEC_ERROR_INVALID_STATE;
    }

    return SCE_AUDIODEC_OK;
}

} // extern "C"
