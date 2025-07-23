#pragma once

/**
 * @file OrbisAudioDecoder.h
 * @brief FFmpeg-based audio decoder header for ShadPS4
 * 
 * This header defines the OrbisAudioDecoder class which provides
 * FFmpeg-based audio decoding capabilities for the ShadPS4 emulator.
 */

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}

#include <string>

namespace ShadPS4::Audio {

/**
 * @brief Decoder information structure
 */
struct DecoderInfo {
    std::string codecName;      // Name of the codec
    int sampleRate;            // Sample rate in Hz
    int channels;              // Number of channels
    AVSampleFormat sampleFormat; // Sample format
    uint64_t channelLayout;    // Channel layout
};

/**
 * @brief FFmpeg-based audio decoder class
 * 
 * This class provides audio decoding functionality using FFmpeg libraries.
 * It supports various audio codecs including M4AAC and handles format conversion
 * to standard PCM output.
 */
class OrbisAudioDecoder {
public:
    /**
     * @brief Constructor
     */
    OrbisAudioDecoder();

    /**
     * @brief Destructor - automatically cleans up resources
     */
    ~OrbisAudioDecoder();

    /**
     * @brief Initialize the decoder with specified codec and parameters
     * @param codecId FFmpeg codec ID (e.g., AV_CODEC_ID_AAC for M4AAC)
     * @param sampleRate Sample rate in Hz
     * @param channels Number of audio channels
     * @return true if initialization successful, false otherwise
     */
    bool initialize(AVCodecID codecId, int sampleRate, int channels);

    /**
     * @brief Decode an audio packet
     * @param packetData Pointer to compressed audio data
     * @param packetSize Size of input packet in bytes
     * @param outputBuffer Pointer to output PCM buffer
     * @param outputBufferSize Size of output buffer in bytes
     * @param outputSize Pointer to store actual output size
     * @return 0 on success, negative error code on failure
     */
    int decodePacket(const uint8_t* packetData, int packetSize,
                    uint8_t* outputBuffer, int outputBufferSize, int* outputSize);

    /**
     * @brief Reset the decoder state
     * @return true if reset successful, false otherwise
     */
    bool reset();

    /**
     * @brief Get decoder information
     * @param info Reference to DecoderInfo structure to fill
     * @return true if info retrieved successfully, false otherwise
     */
    bool getDecoderInfo(DecoderInfo& info) const;

    /**
     * @brief Check if decoder is initialized
     * @return true if initialized, false otherwise
     */
    bool isDecoderInitialized() const { return isInitialized; }

private:
    /**
     * @brief Clean up all allocated resources
     */
    void cleanup();

    // FFmpeg contexts and structures
    AVCodecContext* codecContext;   // Codec context
    SwrContext* swrContext;         // Software resampler context
    const AVCodec* codec;           // Codec instance
    AVFrame* frame;                 // Frame for decoded data
    AVPacket* packet;               // Packet for input data

    // State tracking
    bool isInitialized;             // Initialization state

    // Disable copy constructor and assignment operator
    OrbisAudioDecoder(const OrbisAudioDecoder&) = delete;
    OrbisAudioDecoder& operator=(const OrbisAudioDecoder&) = delete;
};

} // namespace ShadPS4::Audio
