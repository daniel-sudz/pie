#include <unistd.h>

#include <atomic>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "../serial/serial_reciever.hpp"

/* Etch-a-sketch serial processer */
struct EtchaSketchSerialReciever : public serial::SerialReciever {
    /* Linked list node to store potentiometer values */
    struct EtchaSketchPotValNode {
        float left_val = 0;
        float right_val = 0;
        std::atomic<EtchaSketchPotValNode*> next = nullptr;
    };

    EtchaSketchPotValNode* pot_vals_head = new EtchaSketchPotValNode;
    EtchaSketchPotValNode* pot_vals_tail = pot_vals_head;

    std::atomic<int> pot_num_nodes = 0;

    void process_message(std::string msg) {
        // POTL<left pot value>POTR<right pot value>
        if (msg.find("POTL") != std::string::npos && msg.find("POTR") != std::string::npos) {
            int left_pot;
            int right_port;
            sscanf(msg.c_str(), "POTL%dPOTR%d", &left_pot, &right_port);
            process_pot_info((float)left_pot, (float)right_port);
        }
        std::cout << msg << std::endl;
    }

    void process_pot_info(float left_pot, float right_pot) {
    }
};

int main() {
    EtchaSketchSerialReciever serial_reciever;

    /* Main IO loop */
    while (true) {
        serial_reciever.non_block_process_io_loop();
    }
}
