#pragma once

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

namespace serial {
    /* Returns the first /dev/tty.usbmodem device connected to serial */
    std::string get_arduino_serial() {
        std::string arduino_serial;
        for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
            std::string device_name = entry.path().generic_string();
            if (device_name.find("tty.usbmodem") != std::string::npos) {
                arduino_serial = device_name;
            }
        };
        if (arduino_serial.empty()) {
            std::cerr << "[INFO]: Failed to find arduino serial port" << std::endl;
            exit(1);
        }
        std::cout << "[INFO]: Located arduino serial port at " << arduino_serial << std::endl;
        return arduino_serial;
    }

    /* Logs debug info for serial related operations */
    void debug_info(std::string str) {
        std::cout << "[INFO SERIAL]: " << str << std::endl;
    }

    /* Logs debug error for serial related operations */
    void debug_err(std::string str) {
        std::cerr << "[INFO SERIAL]: " << str << std::endl;
    }

    /*  Helper container for periodically logging debug messages.
     *
     *  The contructor takes the logging interval in milliseconds
     *  and calls to log_if_needed(msg) will be dropped if the interval has not elapsed.
     *
     */
    struct DebugLogTimer {
        float log_interval_milliseconds;

        std::chrono::system_clock::time_point last = std::chrono::system_clock::now();

        /* Take a callback that resolves the msg to log.
         * In some cases the message evaluation may be expensive so this helps preserve performance.
         */
        void log_if_needed(std::function<std::string()> get_msg) {
            std::chrono::system_clock::time_point cur = std::chrono::system_clock::now();
            float milliseconds_diff = std::chrono::duration_cast<std::chrono::milliseconds>(cur - last).count();
            if (milliseconds_diff > log_interval_milliseconds) {
                serial::debug_info(get_msg());
                last = std::chrono::system_clock::now();
            }
        }

        DebugLogTimer(float _log_interval_milliseconds) : log_interval_milliseconds(_log_interval_milliseconds) {
        }
    };

    /* Trims message, useful for serial message preprocessing */
    std::string
    trim_message(std::string str) {
        /* https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring */
        str.erase(0, str.find_first_not_of("\t\n\v\f\r "));  // left trim
        str.erase(str.find_last_not_of("\t\n\v\f\r ") + 1);  // right trim
        return str;
    }
}  // namespace serial