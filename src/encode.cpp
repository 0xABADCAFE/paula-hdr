#include "include/types.hpp"
#include "include/cliparameters.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>

typedef RawStaticPCMInput<PCMStream::INT_16, 1, 44100> RawAudioInput;

int main(int argc, const char **argv) {

  CLIParameters params(argc, argv);

  const char* from = params.get("-f");
  const char* to   = params.get("-t");

  if (from) {
    std::printf("From: %s To: %s\n", from, to);
    RawAudioInput source;
    PaulaHDRFileOutput destination;
    if (source.open(from)) {
      destination.setBlockSize(16);
      destination.setFrameSize(16);
      destination.setSampleRate((uint16)source.rate());
      if (destination.open(to)) {
        PaulaHDREncoder encoder(destination.getBlockSize(), destination.getFrameSize());
        encoder.encode(&source, &destination);
      } else {
        std::printf("Failed to open destiation file %s\n", to);
      }
    } else {
      std::printf("Failed to open source file %s\n", from);
    }
  }

  return 0;
}
