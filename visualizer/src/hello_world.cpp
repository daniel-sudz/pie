#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <boost/asio.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "../out/_deps/sdl2-src/include/SDL.h"
#include "AudioFile.h"

/*
   Some helpers for abstracting away playing raw audio on two channels (X,Y)
   Uses the SLD2 video/audio library to be cross-platform
*/
struct RawAudioPlayer {
  /* Print error to cerr and exits code 1*/
 private:
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID output_device;

  static void exit_with_error(std::string err) {
    std::cerr << err << std::endl;
    exit(1);
  }

  /* For some reason SDL2 eats up Ctrl+C events which is super annoying */
  static void process_sdl2_system_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:  // window close icon
          exit_with_error("ctrl+C hit");
      }
    }
  }

 public:
  RawAudioPlayer() {
    // Init SDL2 audio
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) < 0) {
      RawAudioPlayer::exit_with_error("Failed to call SDL_INIT_AUDIO");
    }

    // Configure the audio output type that we want
    want.freq = 192000;          // 192 kHz
    want.format = AUDIO_F32SYS;  // 32-bit floating point samples in native byte order
    want.channels = 2;           // X, Y channels
    want.samples = 1024;         // buffer size
    want.callback = NULL;        // use SDL_QueueAudio instead of callbacks
    want.userdata = NULL;        // use SDL_QueueAudio instead of callbacks

    // Open the audio device
    if ((output_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) < 0) {
      RawAudioPlayer::exit_with_error("Failed to call SDL_OpenAudio");
    }

    // Start audio playback
    SDL_PauseAudioDevice(output_device, 0);

    // Ouput info
    std::cout << "Successfully initialized audio output" << std::endl;
  }

  /* Queue up audio for playback */
  void queue_audio(std::vector<float> X, std::vector<float> Y) {
    // interleave the raw data for the two channels
    std::vector<float> channel_interleave;
    for (int i = 0; i < X.size(); i++) {
      channel_interleave.push_back(X[i]);
      channel_interleave.push_back(Y[i]);
    }
    SDL_QueueAudio(output_device, channel_interleave.data(),
                   channel_interleave.size() * sizeof(channel_interleave[0]));
    RawAudioPlayer::process_sdl2_system_events();
  }

  void render_image(std::vector<float> X, std::vector<float> Y, float render_time) {
    std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

    /* function to return the elapsed time in microseconds */
    auto elapsed_time = [start_time]() -> float {
      std::chrono::high_resolution_clock::time_point cur_time = std::chrono::high_resolution_clock::now();
      auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(cur_time - start_time);
      return elapsed_time.count();
    };

    /* function to calculate the distance between two points */
    auto two_point_distance = [](std::pair<float, float> p1, std::pair<float, float> p2) -> float {
      return sqrt((p1.first - p2.first) * (p1.first - p2.first) + (p1.second - p2.second) * (p1.second - p2.second));
    };

    /* calculate the total segment length to be able to normalize by time */
    float total_segment_length = 0;
    for (int i = 1; i < X.size(); i++) {
      total_segment_length += two_point_distance({X[i - 1], Y[i - 1]}, {X[i], Y[i]});
    }

    int current_render_index = 1;
    float current_render_time = 0;
    while (elapsed_time() < render_time) {
      /* advance the render position based on the time */
      while (current_render_time < elapsed_time() && current_render_index < X.size()) {
        float current_segment_distance = two_point_distance({X[current_render_index - 1], Y[current_render_index - 1]}, {X[current_render_index], Y[current_render_index]});
        float current_segment_time = (current_segment_distance / total_segment_length) * render_time;
        if (current_render_time + current_segment_time >= elapsed_time()) {
          // render out
          float missing_time = elapsed_time() - current_render_time;
          float missing_distance = (missing_time / render_time) * total_segment_length;
          float missing_segment_amount = missing_distance;
          float current_x = (X[current_render_index] - X[current_render_index - 1]) + X[current_render_index - 1];
        } else {
          // advance
          current_render_index++;
        }
      }
    }
  }
};

struct Serial {
  /* file descriptor for arduino serial */
  int serial_descriptor = -1;

  /* current data we have read */
  std::string read_buffer;

  /* returns the path to the arduino serial file */
  std::string
  get_arduino_serial() {
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

  /* read a line from the arduino serial */
  std::string read_line() {
    std::cout << "hi " << std::endl;
    char tmp_buffer[10000];
    int bytes_read = read(serial_descriptor, tmp_buffer, sizeof(tmp_buffer));
    std::cout << "read " << bytes_read << std::endl;
    for (char c : std::string(tmp_buffer, bytes_read)) {
      std::cout << "read_char " << c << std::endl;
    }
    return "";
  }

  /* initialize the serial communication */
  Serial() {
    std::string arduino_serial = get_arduino_serial();
    std::cout << "done init 1" << std::endl;
    serial_descriptor = sys_call_guard(open(arduino_serial.c_str(), O_RDWR | O_NONBLOCK), "failed to open arduino serial file");
    std::cout << "done init n" << std::endl;

    /* configure serial settings */
    struct termios tty;
    std::cout << "done init 2" << std::endl;
    sys_call_guard(tcgetattr(serial_descriptor, &tty), "failed to call tcgetattr");
    std::cout << "done init 3" << std::endl;
    sys_call_guard(cfsetospeed(&tty, B115200), "failed to run cfsetospeed");  // Set the baud rate (e.g., 115200)
    std::cout << "done init 4" << std::endl;
    tty.c_cflag |= (CLOCAL | CREAD);  // Enable receiver and ignore modem control lines
    tty.c_cflag &= ~PARENB;           // No parity
    tty.c_cflag &= ~CSTOPB;           // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;  // 8 data bits
    sys_call_guard(tcsetattr(serial_descriptor, TCSANOW, &tty), "failed to run tcsetattr");
    std::cout << "done init 5" << std::endl;
    std::cout << "done init " << std::endl;
  }
};

int main() {
  AudioFile<double> audioFile;

  Serial serial;
  serial.get_arduino_serial();
  while (true) {
    serial.read_line();
  }

  RawAudioPlayer audio_player;
  /*
  while (true) {
    audio_player.queue_audio(std::vector<float>(1000, 10),
                             std::vector<float>(1000, 10));
    audio_player.queue_audio(std::vector<float>(1000, -10),
                             std::vector<float>(1000, -10));
    // don't burn the CPU too much
    // std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }
  */

  return 0;
}
