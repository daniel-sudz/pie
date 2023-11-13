#pragma once

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

    /* Trims message, useful for serial message preprocessing */
    std::string trim_message(std::string str) {
        /* https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring */
        str.erase(0, str.find_first_not_of("\t\n\v\f\r "));  // left trim
        str.erase(str.find_last_not_of("\t\n\v\f\r ") + 1);  // right trim
        return str;
    }
}  // namespace serial