#pragma once

namespace audio {
/* guards portaudio error */
void guard_portaudio_error(PaError err) {
  if (err != paNoError) {
    fprintf(stderr, "%s", Pa_GetErrorText(err));
    exit(1);
  }
}
}  // namespace audio