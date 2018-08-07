#include "include/types.hpp"
#include "include/cliparameters.hpp"
#include "include/pcmstream.hpp"
#include <cstdio>


int main(int argc, const char **argv) {

  CLIParameters params(argc, argv);

  const char* from = params.get("-f");
  const char* to   = params.get("-t");

  if (from && to) {
    std::printf("From: %s To: %s\n", from, to);

    RawStaticPCMInput<PCMStream::INT_16, 1, 44100> source;

    source.open(from);

  }

  return 0;
}
