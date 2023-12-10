#pragma once

#include <stdio.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "./defaults.hpp"

namespace audio {

    /* Logs debug info for serial related operations */
    void debug_info(std::string str) {
        std::cout << "[INFO AUDIO]: " << str << std::endl;
    }

    /* Logs debug error for serial related operations */
    void debug_err(std::string str) {
        std::cerr << "[INFO AUDIO]: " << str << std::endl;
    }

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
        audio::debug_info("started initializing portaudio");
        audio::guard_portaudio_error(Pa_Initialize(), "audio::portaudio_initialize()");
        audio::debug_info("finished initializing portaudio");
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
        PaDeviceIndex num_devices = Pa_GetDeviceCount();
        PaDeviceIndex the_schiit_box = -1;

        for (int i = 0; i < num_devices; i++) {
            const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
            std::string device_name = device_info->name;
            if (device_name.find("Schiit Modi 3") != std::string::npos) {
                the_schiit_box = i;
            }
        }

        if (the_schiit_box != -1) {
            audio::debug_info("[INFO: AUDIO]: FOUND THE SCHIIT BOX");

            /* SCHIIT BOX settings */
            const PaStreamParameters output_parameters = {
                .device = the_schiit_box,
                .channelCount = 2,
                .sampleFormat = paFloat32,
                .suggestedLatency = PaTime((1e-3) * 15),
                .hostApiSpecificStreamInfo = nullptr};

            /* Check for sample rate compatibility*/
            audio::guard_portaudio_error(Pa_IsFormatSupported(NULL, &output_parameters, audio::sample_rate));
            audio::debug_info("Sample rate of " + std::to_string(audio::sample_rate) + " is supported!!!");

            audio::guard_portaudio_error(
                Pa_OpenStream(
                    /* The stream to open*/
                    stream,
                    /* No input channels */
                    NULL,
                    /* Stereo output */
                    &output_parameters,

                    /* Sample rate of the output*/
                    audio::sample_rate,
                    /*  Frames per buffer, i.e. the number of sample frames that PortAudio will
                     *  request from the callback. Many apps may want to use paFramesPerBufferUnspecified, which
                     *  tells PortAudio to pick the best, possibly changing, buffer size.
                     */
                    (audio::sample_rate / (1 << 8)),

                    /* Stream flag settings*/
                    paNoFlag,

                    /* This is your callback function */
                    streamCallback,

                    /* This is a pointer that will be passed to your callback*/
                    data),
                "audio::open_stream()");
        } else {
            audio::debug_err("COULD NOT FIND SCHIIT BOX!!!!!");
        }
    }

}  // namespace audio