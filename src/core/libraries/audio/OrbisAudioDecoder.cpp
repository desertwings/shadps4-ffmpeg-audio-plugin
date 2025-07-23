/**
 * @file OrbisAudioDecoder.cpp
 * @brief FFmpeg-based audio decoder implementation for ShadPS4
 * 
 * This file implements the core audio decoding functionality using FFmpeg libraries.
 * It provides M4AAC decoding capabilities with proper error handling and logging.
 */

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/opt.h>
    #include <libswresample/swresample.h>
}

#include "OrbisAudioDecoder.h"
#include <iostream>
#include <memory>
#include <cstring>

namespace ShadPS4::Audio {

OrbisAudioDecoder::OrbisAudioDecoder() 
    : codecContext(nullptr)
    , swrContext(nullptr)
    , codec(nullptr)
    , frame(nullptr)
    , packet(nullptr)
    , isInitialized(false) {
}

OrbisAudioDecoder::~OrbisAudioDecoder() {
    cleanup();
}

bool OrbisAudioDecoder::initialize(AVCodecID codecId, int sampleRate, int channels) {
    if (isInitialized) {
        std::cout << "[OrbisAudioDecoder] Already initialized, cleaning up first" << std::endl;
        cleanup();
    }

    // Find the decoder
    codec = avcodec_find_decoder(codecId);
    if (!codec) {
        std::cerr << "[OrbisAudioDecoder] Error: Codec not found for ID " << codecId << std::endl;
        return false;
    }

    std::cout << "[OrbisAudioDecoder] Found codec: " << codec->name << std::endl;

    // Allocate codec context
    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        std::cerr << "[OrbisAudioDecoder] Error: Could not allocate codec context" << std::endl;
        return false;
    }

    // Set codec parameters
    codecContext->sample_rate = sampleRate;
    codecContext->channels = channels;
    codecContext->channel_layout = av_get_default_channel_layout(channels);

    // Open codec
    int ret = avcodec_open2(codecContext, codec, nullptr);
    if (ret < 0) {
        char errorStr[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errorStr, sizeof(errorStr));
        std::cerr << "[OrbisAudioDecoder] Error opening codec: " << errorStr << std::endl;
        cleanup();
        return false;
    }

    // Allocate frame and packet
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        std::cerr << "[OrbisAudioDecoder] Error: Could not allocate frame or packet" << std::endl;
        cleanup();
        return false;
    }

    // Initialize software resampler for format conversion if needed
    swrContext = swr_alloc();
    if (!swrContext) {
        std::cerr << "[OrbisAudioDecoder] Error: Could not allocate resampler context" << std::endl;
        cleanup();
        return false;
    }

    // Configure resampler (convert to standard PCM format)
    av_opt_set_int(swrContext, "in_channel_layout", codecContext->channel_layout, 0);
    av_opt_set_int(swrContext, "out_channel_layout", codecContext->channel_layout, 0);
    av_opt_set_int(swrContext, "in_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_int(swrContext, "out_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", codecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    ret = swr_init(swrContext);
    if (ret < 0) {
        char errorStr[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errorStr, sizeof(errorStr));
        std::cerr << "[OrbisAudioDecoder] Error initializing resampler: " << errorStr << std::endl;
        cleanup();
        return false;
    }

    isInitialized = true;
    std::cout << "[OrbisAudioDecoder] Successfully initialized decoder" << std::endl;
    std::cout << "[OrbisAudioDecoder] Sample rate: " << sampleRate << " Hz, Channels: " << channels << std::endl;
    
    return true;
}

int OrbisAudioDecoder::decodePacket(const uint8_t* packetData, int packetSize, 
                                   uint8_t* outputBuffer, int outputBufferSize, int* outputSize) {
    if (!isInitialized) {
        std::cerr << "[OrbisAudioDecoder] Error: Decoder not initialized" << std::endl;
        return -1;
    }

    if (!packetData || packetSize <= 0 || !outputBuffer || outputBufferSize <= 0 || !outputSize) {
        std::cerr << "[OrbisAudioDecoder] Error: Invalid input parameters" << std::endl;
        return -1;
    }

    *outputSize = 0;

    // Prepare packet
    av_packet_unref(packet);
    packet->data = const_cast<uint8_t*>(packetData);
    packet->size = packetSize;

    // Send packet to decoder
    int ret = avcodec_send_packet(codecContext, packet);
    if (ret < 0) {
        char errorStr[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errorStr, sizeof(errorStr));
        std::cerr << "[OrbisAudioDecoder] Error sending packet to decoder: " << errorStr << std::endl;
        return ret;
    }

    // Receive decoded frame
    ret = avcodec_receive_frame(codecContext, frame);
    if (ret < 0) {
        if (ret == AVERROR(EAGAIN)) {
            // Need more input data
            return 0;
        } else if (ret == AVERROR_EOF) {
            std::cout << "[OrbisAudioDecoder] End of stream reached" << std::endl;
            return ret;
        } else {
            char errorStr[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errorStr, sizeof(errorStr));
            std::cerr << "[OrbisAudioDecoder] Error receiving frame from decoder: " << errorStr << std::endl;
            return ret;
        }
    }

    // Calculate required output buffer size
    int samplesPerChannel = frame->nb_samples;
    int channels = frame->channels;
    int bytesPerSample = 2; // 16-bit PCM
    int requiredSize = samplesPerChannel * channels * bytesPerSample;

    if (requiredSize > outputBufferSize) {
        std::cerr << "[OrbisAudioDecoder] Error: Output buffer too small. Required: " 
                  << requiredSize << ", Available: " << outputBufferSize << std::endl;
        return -2;
    }

    // Convert audio format using swresample
    uint8_t* outputPtr = outputBuffer;
    int convertedSamples = swr_convert(swrContext, &outputPtr, samplesPerChannel,
                                     const_cast<const uint8_t**>(frame->data), samplesPerChannel);

    if (convertedSamples < 0) {
        char errorStr[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(convertedSamples, errorStr, sizeof(errorStr));
        std::cerr << "[OrbisAudioDecoder] Error converting audio format: " << errorStr << std::endl;
        return convertedSamples;
    }

    *outputSize = convertedSamples * channels * bytesPerSample;

    std::cout << "[OrbisAudioDecoder] Decoded frame: sample_rate=" << frame->sample_rate 
              << ", channels=" << frame->channels << ", samples=" << convertedSamples 
              << ", output_size=" << *outputSize << std::endl;

    return 0; // Success
}

bool OrbisAudioDecoder::reset() {
    if (!isInitialized) {
        return false;
    }

    // Flush the decoder
    avcodec_flush_buffers(codecContext);
    
    std::cout << "[OrbisAudioDecoder] Decoder reset successfully" << std::endl;
    return true;
}

void OrbisAudioDecoder::cleanup() {
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = nullptr;
    }

    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }

    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (codecContext) {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }

    codec = nullptr;
    isInitialized = false;

    std::cout << "[OrbisAudioDecoder] Cleanup completed" << std::endl;
}

bool OrbisAudioDecoder::getDecoderInfo(DecoderInfo& info) const {
    if (!isInitialized || !codecContext) {
        return false;
    }

    info.codecName = codec ? codec->name : "Unknown";
    info.sampleRate = codecContext->sample_rate;
    info.channels = codecContext->channels;
    info.sampleFormat = codecContext->sample_fmt;
    info.channelLayout = codecContext->channel_layout;

    return true;
}

} // namespace ShadPS4::Audio
