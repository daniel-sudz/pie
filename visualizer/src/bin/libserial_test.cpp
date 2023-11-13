#include <unistd.h>

#include <filesystem>
#include <iostream>

#include "../../thirdparty/libserial/src/libserial/SerialPort.h"

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

int main() {
    std::string arduino_name = get_arduino_serial();
    LibSerial::SerialPort serial_port;
    try {
        // Open the Serial Port at the desired hardware port.

        serial_port.Open(arduino_name.c_str());
    } catch (const LibSerial::OpenFailed&) {
        std::cerr << "The serial port did not open correctly." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "[INFO]: Opened arduino Port at " << arduino_name << std::endl;

    // Set the baud rate of the serial port.
    serial_port.SetBaudRate(LibSerial::BaudRate::BAUD_115200);

    // Set the number of data bits.
    serial_port.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);

    // Turn off hardware flow control.
    serial_port.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);

    // Disable parity.
    serial_port.SetParity(LibSerial::Parity::PARITY_NONE);

    // Set the number of stop bits.
    serial_port.SetStopBits(LibSerial::StopBits::STOP_BITS_1);

    while (true) {
        // Wait for data to be available at the serial port
        while (!serial_port.IsDataAvailable()) {
            usleep(1000);
        }

        LibSerial::DataBuffer read_buffer(1);

        serial_port.Read(read_buffer, 1, 0);
        for (int i = 0; i < read_buffer.size(); i++) {
            std::cout << read_buffer[i];
        }
    }
}
