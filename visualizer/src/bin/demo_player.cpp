#include "../../thirdparty/portaudio/include/portaudio.h"
#include "../audio/player.hpp"

/* Sample player that just plays the same note */
struct DemoPlayer : public audio::Player<DemoPlayer> {
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

int main() {
    DemoPlayer player;
    Pa_Sleep(1000 * 100);
}