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

    /* Preallocate potentiometer values */

    struct EtchaSketchPotVal {
        accurate_float left_val = 0;
        accurate_float right_val = 0;
    };
    EtchaSketchPotVal pot_vals[1000 * 60 * 60];
    std::atomic<int> pot_num_values = 0;

    /* Check atomic property */
    static_assert(std::atomic<int>::is_always_lock_free, "int should be lock-free to avoid undefined behaviour");

    /* Frequency at which to trace the etch a sketch */
    std::atomic<uint32_t> trace_freqs = 0;

    /* Lookup table for how to decode KEYBOARDNOTES bit field */
    float keyboardnotes_lookup_table[30] = {
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
        246.94,
        //
        261.63,
        311.13,
        369.99,
        440.00,
        //
        277.18,
        329.63,
        392.00,
        466.16,
        //
        293.66,
        349.23,
        415.30,
        493.88};

    void
    process_message(std::string msg) {
        // POTL<left pot value>POTR<right pot value>
        if (msg.find("POTL") != std::string::npos && msg.find("POTR") != std::string::npos) {
            int left_pot;
            int right_pot;
            sscanf(msg.c_str(), "POTL%dPOTR%d", &left_pot, &right_pot);
            pot_value_debug_logger.log_if_needed([left_pot, right_pot, this]() { return ("PotL: " + std::to_string(left_pot) + " PotR: " + std::to_string(right_pot)); });
            /* Make sure some change some happened instead of continuously appending the same value */
            if (pot_num_values == 0 || (std::fabs(accurate_float(left_pot) - pot_vals[pot_num_values - 1].left_val) >= 5) || (std::fabs(accurate_float(right_pot) - pot_vals[pot_num_values - 1].right_val) >= 5)) {
                process_pot_info((accurate_float)left_pot, (accurate_float)right_pot);
            }
        } else if (msg.find("KEYBOARDNOTES") != std::string::npos) {
            int new_trace_freqs;
            sscanf(msg.c_str(), "KEYBOARDNOTES%d", &new_trace_freqs);
            trace_freqs = new_trace_freqs;
        } else if (msg.find("ERASE") != std::string::npos) {
            // restart the program
            exit(0);
        }
        pot_count_debug_logger.log_if_needed([this]() { return "Currently holding " + std::to_string(pot_num_values) + " pot value and playing freq " + std::to_string(trace_freqs) + " frequency"; });
    }

    /* Processes a new update from the potentiometers */
    void process_pot_info(accurate_float left_pot, accurate_float right_pot) {
        /* Set the first value */
        if (pot_num_values == 0) {
            pot_vals[0].left_val = left_pot;
            pot_vals[0].right_val = right_pot;
            pot_num_values = 1;
        }
        /* Set the next value */
        else {
            pot_vals[pot_num_values].left_val = left_pot;
            pot_vals[pot_num_values].right_val = right_pot;
            pot_num_values++;
        }
    }
};

/* Etch-a-sketch osciliscope driver */
struct EtchaSketchPlayer : public audio::Player<EtchaSketchPlayer> {
   public:
    /* Our serial processing is abstracted away here */
    EtchaSketchSerialReciever serial_reciever;

    /* Based on all the samples we have buffered to the DAC, what time are we currently at */
    accurate_float current_buffered_trace_time = 0;

    static int streamCallback(const float* input, float* output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
        /* Get a handle to ourselves */
        EtchaSketchPlayer* self = (EtchaSketchPlayer*)userData;

        self->serial_reciever.timestamp_contribution_logger.log_if_needed([self]() { return " timestamp buffer_trace_time " + std::to_string(self->current_buffered_trace_time); });

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
            /* Calculate the desired position on the Etch-A-Sketch based on the note contributions */
            accurate_float notes_pressed = 0.0;
            accurate_float total_trace_positions_sum = 0.0;

            for (uint32_t note = 0; note < 30; note++) {
                uint32_t note_field = (uint32_t(1) << note);
                if (self->serial_reciever.trace_freqs & note_field) {
                    /* Note is being pressed, calculate contribution */
                    accurate_float note_freq = accurate_float(self->serial_reciever.keyboardnotes_lookup_table[note]);
                    accurate_float note_total_traces = note_freq * self->current_buffered_trace_time;
                    accurate_float note_current_trace_pos = note_total_traces - std::floorf(note_total_traces);

                    /* Triangle instead of saw-tooth */
                    if (((long long)note_total_traces) % 2 == 1) {
                        note_current_trace_pos = (1 - note_current_trace_pos);
                    }

                    total_trace_positions_sum += note_current_trace_pos;
                    notes_pressed += 1.0;
                }
            }
            int desired_trace_position = ((total_trace_positions_sum / notes_pressed) * accurate_float(self->serial_reciever.pot_num_values)) - 1;
            desired_trace_position = std::max(desired_trace_position, 0);
            desired_trace_position = std::min(desired_trace_position, self->serial_reciever.pot_num_values.load() - 1);

            /* Retrieve the pot val that we are currently on */
            accurate_float left_pot_val = self->serial_reciever.pot_vals[desired_trace_position].left_val;
            accurate_float right_pot_val = self->serial_reciever.pot_vals[desired_trace_position].right_val;

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

    void* get_concrete_handle() {
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
