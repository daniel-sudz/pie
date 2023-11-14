#include <unistd.h>

#include <atomic>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "../audio/player.hpp"
#include "../serial/serial_reciever.hpp"

/* Etch-a-sketch serial processer */
struct EtchaSketchSerialReciever : public serial::SerialReciever {
    /* Periodically log the amount of potentiometer values we have */
    serial::DebugLogTimer pot_count_debug_logger = serial::DebugLogTimer(1000);

    /* Linked list node to store potentiometer values */
    struct EtchaSketchPotValNode {
        float left_val = 0;
        float right_val = 0;
        EtchaSketchPotValNode* next = nullptr;
    };

    EtchaSketchPotValNode* pot_vals_head = new EtchaSketchPotValNode;
    EtchaSketchPotValNode* pot_vals_tail = pot_vals_head;
    std::atomic<int> pot_num_values = 0;

    void process_message(std::string msg) {
        // POTL<left pot value>POTR<right pot value>
        if (msg.find("POTL") != std::string::npos && msg.find("POTR") != std::string::npos) {
            int left_pot;
            int right_port;
            sscanf(msg.c_str(), "POTL%dPOTR%d", &left_pot, &right_port);
            process_pot_info((float)left_pot, (float)right_port);
        }
        pot_count_debug_logger.log_if_needed([this]() { return "Currently holding " + std::to_string(pot_num_values) + " pot value"; });
    }

    /* Processes a new update from the potentiometers */
    void process_pot_info(float left_pot, float right_pot) {
        if (pot_num_values == 0) {
            /* Set the first value */
            pot_vals_head->left_val = left_pot;
            pot_vals_head->left_val = right_pot;
            pot_num_values = 1;
        } else {
            /* Set the next value */
            pot_vals_tail->next = new EtchaSketchPotValNode;
            pot_vals_tail->next->left_val = left_pot;
            pot_vals_tail->next->right_val = right_pot;
            /* Move the tail */
            pot_vals_tail = pot_vals_tail->next;
            pot_num_values++;
        }
    }
};

/* Etch-a-sketch osciliscope driver */
struct EtchaSketchPlayer : public audio::Player<EtchaSketchPlayer> {
   public:
    /* Our serial processing is abstracted away here */
    EtchaSketchSerialReciever serial_reciever;

    /* We loop all the pot vals and just play them in sequence */
    int current_pot_idx_node = 0;
    EtchaSketchSerialReciever::EtchaSketchPotValNode* current_pot_node = serial_reciever.pot_vals_head;

    static int streamCallback(const float* input, float* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        /* Get a handle to ourselves */
        EtchaSketchPlayer* self = (EtchaSketchPlayer*)userData;

        /* Fill the buffer */
        for (int i = 0; i < frameCount; i++) {
            /* Reset to beginning on pot vals if we reach the end */
            if (self->current_pot_idx_node >= self->serial_reciever.pot_num_values) {
                self->current_pot_idx_node = 0;
                self->current_pot_node = self->serial_reciever.pot_vals_head;
            }

            /* Retrieve the pot val that we are currently on */
            float left_pot_val = self->current_pot_node->left_val;
            float right_pot_val = self->current_pot_node->right_val;

            /* Convert the pot val into a our sound range [-1, 1]
             * Let's assume the pot range output is linear in [0, 1023]
             */
            float pot_range_max = 1023.f;
            float left_pot_sound = (left_pot_val - (pot_range_max / 2)) / (pot_range_max / 2);
            float right_pot_sound = (right_pot_val - (pot_range_max / 2)) / (pot_range_max / 2);

            /* Actually fill the sound buffer */
            output[i * 2] = left_pot_sound;
            output[i * 2 + 1] = right_pot_sound;

            /* Increment the pot val that we read */
            self->current_pot_idx_node++;
            self->current_pot_node = self->current_pot_node->next;
        }

        /* All is good */
        return 0;
    }
};

int main() {
    EtchaSketchPlayer driver;

    /* Main IO loop */
    while (true) {
        driver.serial_reciever.non_block_process_io_loop();
    }
}
