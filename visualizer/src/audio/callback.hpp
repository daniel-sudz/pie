#pragma once

#include "../../thirdparty/portaudio/include/portaudio.h"
#include "./open_stream.hpp"

namespace audio {
/* Data is writen out
 *
 */
typedef struct
{
  float left_phase;
  float right_phase;
} paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
 */
static int patestCallback(const void* inputBuffer, void* outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData) {
  /* Cast data passed through stream to our structure. */
  paTestData* data = (paTestData*)userData;
  float* out = (float*)outputBuffer;

  /* We don't have any incoming audio data */
  (void)inputBuffer;

  for (int i = 0; i < framesPerBuffer; i++) {
  }

  /* All good */
  return 0;
}
}  // namespace audio
