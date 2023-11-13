#include <unistd.h>

#include <filesystem>
#include <iostream>

#include "../serial/serial_reciever.hpp"

/* Sample serial reciever that just echos all messages */
struct DemoSerialReciever : public serial::SerialReciever {
    void process_message(std::string msg) {
        std::cout << msg << std::endl;
    }
};

int main() {
    DemoSerialReciever reciever;
    while (true) {
        reciever.non_block_process_io_loop();
    }
}
