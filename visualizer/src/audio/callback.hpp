#pragma once

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "./open_stream.hpp"

namespace audio {

    /* This routine will be called by the PortAudio engine when audio is needed.
     * It may called at interrupt level on some machines so don't do anything
     * that could mess up the system like calling malloc() or free().
     */

    /* Sample callback implementation that just plays a high note.
     * Note that sound is written out in interleaved L/R format in the range [-1,1].
     */
    static int sampleCallback(const void* _inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        float* out = (float*)outputBuffer;
        for (int i = 0; i < framesPerBuffer; i++) {
            out[i * 2] = 1;        // Set left sound
            out[(i * 2) + 1] = 1;  // Set right sound
        }

        /* All good */
        return 0;
    }
}  // namespace audio
