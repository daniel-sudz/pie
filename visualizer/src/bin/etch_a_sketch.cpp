#include <unistd.h>

#include <algorithm>
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
    serial::DebugLogTimer pot_value_debug_logger = serial::DebugLogTimer(1000);
    serial::DebugLogTimer tmp_logger_1 = serial::DebugLogTimer(1000);

    /* Linked list node to store potentiometer values */
    struct EtchaSketchPotValNode {
        float left_val = 0;
        float right_val = 0;
        EtchaSketchPotValNode* next = nullptr;
    };

    /* Frequency at which to trace the etch a sketch */
    float trace_freq = 1e4;

    EtchaSketchPotValNode* pot_vals_head = new EtchaSketchPotValNode;
    EtchaSketchPotValNode* pot_vals_tail = pot_vals_head;
    std::atomic<int> pot_num_values = 0;
    static_assert(std::atomic<int>::is_always_lock_free, "int should be lock-free to avoid undefined behaviour");

    void process_message(std::string msg) {
        // POTL<left pot value>POTR<right pot value>
        if (msg.find("POTL") != std::string::npos && msg.find("POTR") != std::string::npos) {
            int left_pot;
            int right_pot;
            sscanf(msg.c_str(), "POTL%dPOTR%d", &left_pot, &right_pot);
            pot_value_debug_logger.log_if_needed([left_pot, right_pot, this]() { return ("PotL: " + std::to_string(left_pot) + " PotR: " + std::to_string(right_pot) + " TailL: " + std::to_string(pot_vals_tail->left_val) + " TailR: " + std::to_string(pot_vals_tail->right_val)); });
            /* Make sure some change some happened instead of continuously appending the same value */
            if ((std::abs(left_pot - pot_vals_tail->left_val) >= 5) || (std::abs(right_pot - pot_vals_tail->right_val) >= 5)) {
                tmp_logger_1.log_if_needed([this, left_pot, right_pot]() { return std::to_string(std::abs(left_pot - pot_vals_tail->left_val)) + " r diff " + std::to_string(std::abs(right_pot - pot_vals_tail->right_val)); });
                process_pot_info((float)left_pot, (float)right_pot);
            }
        }
        pot_count_debug_logger.log_if_needed([this]() { return "Currently holding " + std::to_string(pot_num_values) + " pot value"; });
    }

    /* Processes a new update from the potentiometers */
    void process_pot_info(float left_pot, float right_pot) {
        /* Set the first value */
        if (pot_num_values == 0) {
            pot_vals_head->left_val = left_pot;
            pot_vals_head->right_val = right_pot;
            pot_num_values = 1;
        }
        /* Set the next value */
        else {
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

    /* Based on current_pot_idx_node, how much time have we currently traced */
    float current_trace_time = 0;

    /* Based on all the samples we have buffered to the DAC, what time are we currently at */
    float current_buffered_trace_time = 0;

    static int streamCallback(const float* input, float* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        /* Get a handle to ourselves */
        EtchaSketchPlayer* self = (EtchaSketchPlayer*)userData;

        /* Guard on the case when we have zero pot value readings as there is nothing to play */
        if (self->serial_reciever.pot_num_values == 0) {
            std::fill_n(output, frameCount, 0);
            return 0;
        }

        /* -------------  self->serial_reciever.pot_num_values >= 1 after the guard ------------- */

        /* Fill the buffer */
        for (int i = 0; i < frameCount; i++) {
            /* Scan the trace buffer until we are at the correct play time */
            while (self->current_trace_time < self->current_buffered_trace_time) {
                /* Move the trace buffer by one */
                self->current_pot_idx_node++;
                self->current_pot_node = self->current_pot_node->next;

                /* Loop if we reach the end of the trace buffer */
                if (self->current_pot_node == nullptr) {
                    self->current_pot_idx_node = 0;
                    self->current_pot_node = self->serial_reciever.pot_vals_head;
                }
                /* Increment the trace time */
                self->current_trace_time += (1.0 / self->serial_reciever.pot_num_values) * (1.0 / self->serial_reciever.trace_freq);
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

            /* Increment the amount of buffer time that we have traced */
            self->current_buffered_trace_time += (1.0 / audio::sample_rate);
        }
        /* All is good */
        return 0;
    }

    void*
    get_concrete_handle() {
        return this;
    }
};

int main() {
    EtchaSketchPlayer driver;
    driver.init_player();

    /* Main IO loop */
    while (true) {
        driver.serial_reciever.non_block_process_io_loop();
    }
}
