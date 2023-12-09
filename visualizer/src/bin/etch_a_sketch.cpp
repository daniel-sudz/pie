#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "../audio/player.hpp"
#include "../serial/serial_reciever.hpp"

typedef long double accurate_float;

/* Etch-a-sketch serial processer */
struct EtchaSketchSerialReciever : public serial::SerialReciever {
    /* Periodically log the amount of potentiometer values we have */
    serial::DebugLogTimer pot_count_debug_logger = serial::DebugLogTimer(1000);
    serial::DebugLogTimer pot_value_debug_logger = serial::DebugLogTimer(1000);
    serial::DebugLogTimer timestamp_contribution_logger = serial::DebugLogTimer(1000);
    serial::DebugLogTimer tmp_logger_1 = serial::DebugLogTimer(1000);

    /* Linked list node to store potentiometer values */
    struct EtchaSketchPotValNode {
        accurate_float left_val = 0;
        accurate_float right_val = 0;
        EtchaSketchPotValNode* next = nullptr;
    };

    /* Frequency at which to trace the etch a sketch */
    std::atomic<uint32_t> trace_freqs = 0;

    EtchaSketchPotValNode* pot_vals_head = new EtchaSketchPotValNode;
    EtchaSketchPotValNode* pot_vals_tail = pot_vals_head;
    std::atomic<int> pot_num_values = 0;
    static_assert(std::atomic<int>::is_always_lock_free, "int should be lock-free to avoid undefined behaviour");

    /* Lookup table for how to decode KEYBOARDNOTES bit field */
    keyboardnotes_lookup_table = {
        130.81,
        155.56,
        185.00,
        220.00,
        //
        138.59,
        164.81,
        196.00,
        233.08,
        //
        146.83,
        174.61,
        207.65,
        246.94
        //
        261.63,
        311.13,
        369.99,
        440.00
        //
        277.18,
        329.63,
        392.00,
        466.16
        //
        293.66,
        349.23,
        415.30,
        493.88
    }

    void
    process_message(std::string msg) {
        // POTL<left pot value>POTR<right pot value>
        if (msg.find("POTL") != std::string::npos && msg.find("POTR") != std::string::npos) {
            int left_pot;
            int right_pot;
            sscanf(msg.c_str(), "POTL%dPOTR%d", &left_pot, &right_pot);
            pot_value_debug_logger.log_if_needed([left_pot, right_pot, this]() { return ("PotL: " + std::to_string(left_pot) + " PotR: " + std::to_string(right_pot) + " TailL: " + std::to_string(pot_vals_tail->left_val) + " TailR: " + std::to_string(pot_vals_tail->right_val)); });
            /* Make sure some change some happened instead of continuously appending the same value */
            if ((std::fabs(accurate_float(left_pot) - pot_vals_tail->left_val) >= 5) || (std::fabs(accurate_float(right_pot) - pot_vals_tail->right_val) >= 5)) {
                tmp_logger_1.log_if_needed([this, left_pot, right_pot]() { return std::to_string(std::fabs(accurate_float(left_pot) - pot_vals_tail->left_val)) + " r diff " + std::to_string(std::fabs(accurate_float(right_pot) - pot_vals_tail->right_val)); });
                process_pot_info((accurate_float)left_pot, (accurate_float)right_pot);
            }
        } else if (msg.find("KEYBOARDNOTES") != std::string::npos) {
            int new_trace_freqs;
            sscanf(msg.c_str(), "KEYBOARDNOTES%d", &new_trace_freqs);
            trace_freqs = new_trace_freqs;
        }
        pot_count_debug_logger.log_if_needed([this]() { return "Currently holding " + std::to_string(pot_num_values) + " pot value and playing freq " + std::to_string(new_trace_freqs) + " frequency"; });
    }

    /* Processes a new update from the potentiometers */
    void process_pot_info(accurate_float left_pot, accurate_float right_pot) {
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
    accurate_float current_trace_time = 0;

    /* Based on all the samples we have buffered to the DAC, what time are we currently at */
    accurate_float current_buffered_trace_time = 0;

    static int streamCallback(const float* input, float* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        /* Get a handle to ourselves */
        EtchaSketchPlayer* self = (EtchaSketchPlayer*)userData;

        self->serial_reciever.timestamp_contribution_logger.log_if_needed([self]() { return "timestamps current_trace_time " + std::to_string(self->current_trace_time) + " timestamp buffer_trace_time " + std::to_string(self->current_buffered_trace_time); });

        /* Guard on the case when we have zero pot value readings as there is nothing to play */
        if (self->serial_reciever.pot_num_values == 0) {
            std::fill_n(output, frameCount * 2, 0);
            return 0;
        }

        /* Guard on the case where the frequency is zero (ie. no note is being pressed on the keyboard) */
        if (self->serial_reciever.trace_freqs == 0) {
            std::fill_n(output, frameCount * 2, 0);
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

                /* Increment the trace times */
                for (uint32_t note = 0; note < 30; note++) {
                    uint32_t note_field = (uint32_t(1) << note);
                    if (self->serial_reciever.trace_freqs & note_field) {
                        accurate_float note_freq = accurate_float(keyboardnotes_lookup_table[note]);
                        accurate_float trace_contribution = (accurate_float(1) / (accurate_float(self->serial_reciever.pot_num_values) * note_freq));
                        self->current_trace_time += trace_contribution;
                    }
                }
            }

            /* Retrieve the pot val that we are currently on */
            accurate_float left_pot_val = self->current_pot_node->left_val;
            accurate_float right_pot_val = self->current_pot_node->right_val;

            /* Convert the pot val into a our sound range [-1, 1]
             * Let's assume the pot range output is linear in [0, 1023]
             */
            accurate_float pot_range_max = 1023.f;
            accurate_float left_pot_sound = (left_pot_val - (pot_range_max / 2)) / (pot_range_max / 2);
            accurate_float right_pot_sound = (right_pot_val - (pot_range_max / 2)) / (pot_range_max / 2);

            /* Actually fill the sound buffer */
            output[i * 2] = left_pot_sound;
            output[i * 2 + 1] = right_pot_sound;

            /* Increment the amount of buffer time that we have traced */
            self->current_buffered_trace_time += (accurate_float(1) / accurate_float(audio::sample_rate));
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
