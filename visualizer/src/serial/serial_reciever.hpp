#pragma once

#include <string>
#include <vector>

#include "../../thirdparty/libserial/src/libserial/SerialPort.h"
#include "./util.hpp"

namespace serial {

    /* An abstract class container for recieving events from the adruino serial.
     *
     * Implementations should implement the callback process_message(std::string serial_msg) to recieve data
     *      and periodically call non_block_process_io_loop() to process the IO loop
     */
    struct SerialReciever {
       private:
        /* Libserial device */
        LibSerial::SerialPort serial_port;

        /* Serial read buffers */
        LibSerial::DataBuffer read_raw_buffer = LibSerial::DataBuffer(1);
        std::string read_line_buffer;

        /* Implementations should implement the callback process_message(std::string serial_msg) to recieve data */
        virtual void process_message(std::string serial_msg) = 0;

       public:
        /* Binds the SerialReciever to the device specified or default device if not specified.
         * serial_port is then configured based on Arduino Uno defaults.
         */
        SerialReciever(std::string arduino_serial_device = get_arduino_serial()) {
            try {
                serial_port.Open(arduino_serial_device.c_str());
            } catch (const LibSerial::OpenFailed&) {
                serial::debug_err("The serial port did not open correctly.");
                exit(1);
            }
            serial::debug_info("The serial port has opened succesfully");

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
        }

        /* Non-blocking call to process serial IO
         * This should be called as frequently as possible to keep the serial IO going.
         */
        void non_block_process_io_loop() {
            /* Read a single char is availiable */
            if (serial_port.IsDataAvailable()) {
                serial_port.Read(read_raw_buffer, 1, 0);

                /* If we have a complete message, fire the message reciever callback */
                if (read_raw_buffer[0] == '\n') {
                    process_message(read_line_buffer);
                    read_line_buffer.clear();
                } else {
                    read_line_buffer += read_raw_buffer[0];
                }
            }
        };
    };

}  // namespace serial
