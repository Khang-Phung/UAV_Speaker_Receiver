#include "Opus.hpp"
#include "config.hpp"
#include <cstdio>
#include <opus/opus.h>

static OpusDecoder* opusDecoder = nullptr;

bool Opus::init()
{
    int error = OPUS_OK;

    opusDecoder = opus_decoder_create(
        SAMPLE_RATE,
        CHANNELS,
        &error
    );

    if (error != OPUS_OK || opusDecoder == nullptr)
    {
        std::printf(
            "Opus init failed: %s\n",
            opus_strerror(error)
        );

        return false;
    }

    // std::printf("Opus decoder ready\n");

    return true;
}

int Opus::decode(const uint8_t* data,int len)
{ 
    if (opusDecoder == nullptr)
    {
        std::printf("Opus decoder not initialized\n");
        return -1;
    }

    int samples = opus_decode(
        opusDecoder,
        data,
        len,
        _pcmBuffer,
        FRAME_SIZE,
        0
    );

    if (samples < 0)
    {
        std::printf(
            "Opus decode failed: %s\n",
            opus_strerror(samples)
        );

        return samples;
    }

    return samples;
}

int16_t* Opus::getBuffer()
{
    return _pcmBuffer;
}

void Opus::low_volume()
{
    for (int i = 0; i < FRAME_SIZE; i++)
    {
        _pcmBuffer[i] /= 1;
    }
}

size_t Opus::getBufferSize()
{
    return sizeof(_pcmBuffer);
}