#pragma once

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "./util.hpp"

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
        void* Userdata;

        inline static bool PORTAUDIO_INITIALIZED = false;

        static int __streamCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
            return T::streamCallback((float*)input, (float*)output, frameCount, timeInfo, statusFlags, userData);
        }

        virtual void* get_concrete_handle() = 0;

       public:
        void init_player() {
            if (!PORTAUDIO_INITIALIZED) {
                audio::portaudio_initialize();
                PORTAUDIO_INITIALIZED = true;
            }
            Userdata = get_concrete_handle();
            printf("This handle :  %p\n", Userdata);
            audio::portaudio_open_stream(&stream, __streamCallback, Userdata);
            audio::portaudio_start_stream(stream);
            printf("Stream pointer:  %p\n", stream);
        }
        Player() {
        }
    };

}  // namespace audio
