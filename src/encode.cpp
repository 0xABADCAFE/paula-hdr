#include "include/types.hpp"
#include "include/cliparameters.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>
#include <cstdlib>

typedef RawStaticPCMInput<PCMStream::INT_16, 1, 44100> RawAudioInput;

int main(int argc, const char **argv) {

  CLIParameters params(argc, argv);

  const char* from = params.get("-s");
  const char* to   = params.get("-o");

  if (from) {
    RawAudioInput source;
    PaulaHDRFileOutput destination;
    if (source.open(from)) {
      destination.setBlockSize(std::atoi(params.get("-b", "16")));
      destination.setFrameSize(std::atoi(params.get("-f", "16")));
      destination.setSampleRate((uint16)source.rate());

      if (destination.open(to)) {

        std::printf(
          "Encoding from: %s to: %s, block size: %d and frame size: %d\n",
          from,
          to,
          destination.getBlockSize(),
          destination.getFrameSize()
        );

        PaulaHDREncoder encoder(destination.getFrameSize(), destination.getBlockSize());
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
