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

        inline static bool PORTAUDIO_INITIALIZED = false;

        static int __streamCallback(const void* input, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
            return T::streamCallback((float*)input, (float*)output, frameCount, timeInfo, statusFlags, userData);
        }

       public:
        Player() {
            if (!PORTAUDIO_INITIALIZED) {
                audio::portaudio_initialize();
                PORTAUDIO_INITIALIZED = true;
            }
            printf("Value:  %p\n", stream);
            audio::portaudio_open_stream(&stream, __streamCallback, this);
            printf("Value:  %p\n", stream);
            audio::portaudio_start_stream(stream);
        }
    };

    /* Sample player that just plays the same note */
    struct DemoPlayer : public Player<DemoPlayer> {
        float left_phase = 0;
        float right_phase = 0;

        static int streamCallback(const float* input, float* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
            /* Get a handle to ourselves */
            DemoPlayer* self = (DemoPlayer*)userData;

            /* Fill the buffer */
            for (int i = 0; i < frameCount; i++) {
                output[i * 2] = self->left_phase;         // Set left sound
                output[(i * 2) + 1] = self->right_phase;  // Set right sound
            }

            /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
            self->left_phase += 0.01f;

            /* When signal reaches top, drop back down. */
            if (self->left_phase >= 1.0f) {
                self->left_phase -= 2.0f;
            }

            /* higher pitch so we can distinguish left and right. */
            self->right_phase += 0.03f;
            if (self->right_phase >= 1.0f) {
                self->right_phase -= 2.0f;
            }

            std::cerr << "call: " << frameCount << std::endl;

            /* All good */
            return 0;
        }
    };

}  // namespace audio
