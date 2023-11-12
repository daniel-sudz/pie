#pragma once

namespace audio {

    /* Guards portaudio error */
    void guard_portaudio_error(PaError err) {
        if (err != paNoError) {
            fprintf(stderr, "%s", Pa_GetErrorText(err));
            exit(1);
        }
    }

    /* Intializes the portaudio library */
    void portaudio_initialize() {
        audio::guard_portaudio_error(Pa_Initialize());
    }

    /* Terminates the portaudio library */
    void portaudio_terminate() {
        audio::guard_portaudio_error(Pa_Terminate());
    }

    /* */

}  // namespace audio