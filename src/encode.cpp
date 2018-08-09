#include "include/types.hpp"
#include "include/cliparameters.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>

typedef RawStaticPCMInput<PCMStream::INT_16, 1, 44100> RawAudioInput;

int main(int argc, const char **argv) {

  CLIParameters params(argc, argv);

  const char* from = params.get("-f");
  const char* to   = "<NULL>";//params.get("-t");

  if (from) {
    std::printf("From: %s To: %s\n", from, to);
    RawAudioInput source;
    if (source.open(from)) {
      PaulaHDREncoder encoder(16, 16);
      encoder.encode(&source, 0);
    }
  }

  return 0;
}
