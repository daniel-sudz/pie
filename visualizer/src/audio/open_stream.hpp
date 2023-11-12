#pragma once

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "./defaults.hpp"
#include "./util.hpp"

namespace audio {
    /* Opens a portaudio stream with a given callback and data pointer */
    void open_stream(PaStream* stream, PaStreamCallback* streamCallback, void* data) {
        audio::guard_portaudio_error(
            Pa_OpenDefaultStream(
                /* The stream to open*/
                &stream,
                /* No input channels */
                0,
                /* Stereo output */
                2,
                /* 32 bit floating point output */
                paFloat32,
                /* Sample rate of the output*/
                audio::sample_rate,
                /*  Frames per buffer, i.e. the number of sample frames that PortAudio will
                 *  request from the callback. Many apps may want to use paFramesPerBufferUnspecified, which
                 *  tells PortAudio to pick the best, possibly changing, buffer size.
                 */
                256,
                /* This is your callback function */
                streamCallback,
                /* This is a pointer that will be passed to your callback*/
                &data));
    }
}  // namespace audio
