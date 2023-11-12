#pragma once

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "./open_stream.hpp"

namespace audio {

    template <class T>
    /*
     *   An abstract class container for storing everything needed to play a stream by the portaudio API
     *
     *   Derived classes should implement audio::streamCallback
     *      This routine will be called by the PortAudio engine when audio is needed.
     *      It may called at interrupt level on some machines so don't do anything
     *      that could mess up the system like calling malloc() or free().
     */
    struct Player {
       private:
        /* The stream for this player. Do not modify yourself. */
        PaStream* stream;

        static int __streamCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
            T::streamCallback(input, output, frameCount, timeInfo, statusFlags, userData);
        }

       public:
        Player() {
            audio::open_stream(stream, __streamCallback, this);
        }
    };

}  // namespace audio
