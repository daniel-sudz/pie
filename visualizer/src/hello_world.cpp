#include <chrono>
#include <iostream>
#include <string>
#include <thread>

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
    want.freq = 192000;  // 192 kHz
    want.format =
        AUDIO_F32SYS;      // 32-bit floating point samples in native byte order
    want.channels = 2;     // X, Y channels
    want.samples = 1024;   // buffer size
    want.callback = NULL;  // use SDL_QueueAudio instead of callbacks
    want.userdata = NULL;  // use SDL_QueueAudio instead of callbacks

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

int main() {
  AudioFile<double> audioFile;

  RawAudioPlayer audio_player;
  while (true) {
    audio_player.queue_audio(std::vector<float>(1000, 10),
                             std::vector<float>(1000, 10));
    audio_player.queue_audio(std::vector<float>(1000, -10),
                             std::vector<float>(1000, -10));
    // don't burn the CPU too much
    // std::this_thread::sleep_for (std::chrono::milliseconds(10));
  }

  return 0;
}
