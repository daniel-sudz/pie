#include "AudioFile.h"
#include<iostream>
#include <string>

#include "../out/_deps/sdl2-src/include/SDL.h"


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
  
  public: 
    RawAudioPlayer() {
        // Init SDL2 audio
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
          RawAudioPlayer::exit_with_error("Failed to call SDL_INIT_AUDIO");
        }

        // Configure the audio output type that we want
        want.freq = 192000; // 192 kHz
        want.format = AUDIO_F32SYS; // 32-bit floating point samples in native byte order
        want.channels = 2; // X, Y channels
        want.samples = 1024; // buffer size
        want.callback = NULL; // use SDL_QueueAudio instead of callbacks
        want.userdata = NULL; // use SDL_QueueAudio instead of callbacks

        // Open the audio device 
        if ((output_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) < 0) {
            RawAudioPlayer::exit_with_error("Failed to call SDL_OpenAudio");
        }

        // Start audio playback 
        SDL_PauseAudio(0);
    }

    /* Queue up audio for playback */
    void queue_audio(std::vector<float> X, std::vector<float> Y)
    {
        std::vector<float> channel_interleave;
        for (int i = 0; i < X.size();i ++) {
            channel_interleave.push_back(X[i]);
            channel_interleave.push_back(Y[i]);
        }
        SDL_QueueAudio(output_device, channel_interleave.data(), channel_interleave.size() * sizeof(channel_interleave[0]));
    }
};

int main(){
  AudioFile<double> audioFile;

  RawAudioPlayer audio_player;

  return 0;
}
