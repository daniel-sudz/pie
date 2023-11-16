#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>

#include "./defaults.hpp"

namespace audio {

    /* Guards portaudio error */
    void guard_portaudio_error(PaError err, std::string cutom_err = "") {
        if (err != paNoError) {
            std::cerr << "[" << cutom_err << "]: ";
            fprintf(stderr, "%s\n", Pa_GetErrorText(err));
            exit(1);
        }
    }

    /* Intializes the portaudio library */
    void portaudio_initialize() {
        audio::guard_portaudio_error(Pa_Initialize(), "audio::portaudio_initialize()");
    }

    /* Terminates the portaudio library */
    void portaudio_terminate() {
        audio::guard_portaudio_error(Pa_Terminate(), "audio::portaudio_terminate()");
    }

    /* Starts a portaudio stream */
    void portaudio_start_stream(PaStream* stream) {
        audio::guard_portaudio_error(Pa_StartStream(stream), "audio::portaudio_start_stream()");
    }

    /* Opens a portaudio stream with a given callback and data pointer */
    void portaudio_open_stream(PaStream** stream, PaStreamCallback* streamCallback, void* data) {
        audio::guard_portaudio_error(
            Pa_OpenDefaultStream(
                /* The stream to open*/
                stream,
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
                paFramesPerBufferUnspecified,
                /* This is your callback function */
                streamCallback,
                /* This is a pointer that will be passed to your callback*/
                data),
            "audio::open_stream()");
    }

}  // namespace audio