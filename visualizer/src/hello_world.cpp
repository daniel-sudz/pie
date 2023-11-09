#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <deque>
#include <filesystem>
#include <iostream>
#include <string>

#include "../out/_deps/sdl2-src/include/SDL.h"
#include "./raw_audio_player.hpp"
#include "AudioFile.h"
#include "boost/algorithm/string.hpp"
#include "boost/asio.hpp"

struct Serial {
  /* file descriptor for arduino serial */
  boost::asio::io_service io;
  boost::asio::serial_port arduino_serial_port;
  RawAudioPlayer audio_player;

  /* timers for io */
  boost::asio::deadline_timer parse_serial = boost::asio::deadline_timer(io, boost::posix_time::milliseconds(1));
  boost::asio::deadline_timer note_player = boost::asio::deadline_timer(io, boost::posix_time::milliseconds(1));

  /* current data we have read */
  std::deque<char> read_buffer;
  char tmp_buffer[1000];

  /* current state */
  std::string last_note_read;

  /* returns the path to the arduino serial file */
  std::string get_arduino_serial() {
    std::string arduino_serial;
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
      std::string device_name = entry.path().generic_string();
      if (device_name.find("tty.usbmodem") != std::string::npos) {
        arduino_serial = device_name;
      }
    };
    if (arduino_serial.empty()) {
      std::cerr << "failed to find arduino serial port" << std::endl;
      exit(1);
    }
    std::cout << "located arduino serial port at " << arduino_serial << std::endl;
    return arduino_serial;
  }

  /* guards against system call error */
  int sys_call_guard(int code, std::string err) {
    if (code == -1) {
      std::cerr << err << std::endl;
      std::cerr << "Error: " << std::strerror(errno) << std::endl;
      exit(1);
    }
    return code;
  }

  /* queue continuous async read */
  void queue_async_read() {
    arduino_serial_port.async_read_some(boost::asio::mutable_buffer(tmp_buffer, sizeof(tmp_buffer)), [this](const boost::system::error_code& error, size_t bytes_read) {
      for (int i = 0; i < bytes_read; i++) {
        read_buffer.push_back(tmp_buffer[i]);
      }
      queue_async_read();
    });
  }

  /* serial parse timer */
  void queue_parse_timer() {
    parse_serial.async_wait([this](const boost::system::error_code&) {
      if (std::find(read_buffer.begin(), read_buffer.end(), '\n') != read_buffer.end()) {
        std::string ret_line;
        while (read_buffer.front() != '\n') {
          ret_line += read_buffer.front();
          read_buffer.pop_front();
        }
        read_buffer.pop_front();
        boost::trim(ret_line);
        last_note_read = ret_line;
        std::cout << ret_line << std::endl;
      }

      parse_serial = boost::asio::deadline_timer(io, boost::posix_time::milliseconds(1));
      queue_parse_timer();
    });
  }

  void queue_note_player_timer() {
    note_player.async_wait([this](const boost::system::error_code&) {
      int X = -10;
      int Y = -10;
      if (last_note_read == "C4") {
        X = 10;
        Y = 10;
        std::cout << "hit" << last_note_read << std::endl;

      } else if (last_note_read == "D4") {
        X = 20;
        Y = 20;
      } else if (last_note_read == "E4") {
        X = 30;
        Y = 30;
      } else if (last_note_read == "F4") {
        X = 50;
        Y = 50;
      }
      std::cout << "last note" << last_note_read << std::endl;
      audio_player.queue_audio(std::vector<float>(100, X),
                               std::vector<float>(100, Y));
      note_player = boost::asio::deadline_timer(io, boost::posix_time::milliseconds(1));
      queue_note_player_timer();
    });
  }

  /* queue all async operations */
  void queue_async_all() {
    queue_async_read();
    queue_parse_timer();
    queue_note_player_timer();
  }

  /* read a line from the arduino serial */
  std::string read_line() {
    return "";
  }

  /* initialize the serial communication */
  Serial() : arduino_serial_port(io, get_arduino_serial()) {
    arduino_serial_port.set_option(boost::asio::serial_port::baud_rate(115200));  // Set the same baud rate as Arduino
  }
};

int main() {
  AudioFile<double> audioFile;

  Serial serial;
  serial.queue_async_all();
  serial.io.run();

  /*
while (true) {

  audio_player.queue_audio(std::vector<float>(1000, -10),
                           std::vector<float>(1000, -10));
  // don't burn the CPU too much
  // std::this_thread::sleep_for (std::chrono::milliseconds(10));
}
*/

  return 0;
}
