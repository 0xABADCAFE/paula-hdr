#include "include/types.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>
#include <cstring>

PaulaHDREncoder::PaulaHDREncoder(size_t reqFrameSize, size_t reqBlockSize) :
  readPCMBuffer(0),
  writePCMBuffer(0),
  writeVolBuffer(0)
{
  // Frame Size must be an even number of samples between MIN_FRAMESIZE and MAX_FRAMESIZE
  frameSize = (reqFrameSize & ~1);
  frameSize = frameSize < MIN_FRAMESIZE ?
    MIN_FRAMESIZE : frameSize > MAX_FRAMESIZE ?
      MAX_FRAMESIZE : frameSize;
  blockSize = reqBlockSize < MIN_BLOCKSIZE ?
    MIN_BLOCKSIZE : reqBlockSize > MAX_BLOCKSIZE ?
      MAX_BLOCKSIZE : reqBlockSize;

  bufferSize     = frameSize * blockSize;

  std::fprintf(
    stderr,
    "Frame Length %d, Block Length %d, BufferLength %d\n",
    (int)frameSize,
    (int)blockSize,
    (int)bufferSize
  );

  readPCMBuffer  = new int16[bufferSize];
  writePCMBuffer = new int8[bufferSize];
  writeVolBuffer = new uint8[blockSize];
}

PaulaHDREncoder::~PaulaHDREncoder() {
  delete[] readPCMBuffer;
  delete[] writePCMBuffer;
  delete[] writeVolBuffer;
}

size_t PaulaHDREncoder::encode(PCMInput* input, std::FILE* output) {

  if (!input/*|| !output*/) {
    std::fprintf(stderr, "No input stream available\n");
    return 0;
  }

  if (input->channels() != 1) {
    std::fprintf(stderr, "Only mono input data is supported\n");
    return 0;
  }

  input->start();
  size_t
    samplesRead = 0,
    lastRead    = 0;

  do {
    lastRead = encodeBlock(input, output);
    samplesRead += lastRead;
  } while (lastRead == bufferSize);

  std::fprintf(stderr, "Total Samples: %d\n", (int)samplesRead);

  return samplesRead;
}

size_t PaulaHDREncoder::encodeBlock(PCMInput* input, std::FILE* output) {
  size_t totSamples    = input->read(readPCMBuffer, bufferSize);
  if (totSamples > 0 ) {
    // Reset all our buffers
    writePCMBufferOffset = 0;
    writeVolBufferOffset = 0;
    std::memset(writePCMBuffer, 0, bufferSize);
    std::memset(writeVolBuffer, 0, blockSize);

    size_t totalRemaining = totSamples;

  }
  return totSamples;
}

uint8 PaulaHDREncoder::encodeFrame(const int16* input, int8* output, size_t length) {

  return 0;
}

const float PaulaHDREncoder::defaultVol[64] = {
  64.00000000, 32.00000000, 21.33333333, 16.00000000, 12.80000000, 10.66666667, 9.14285714, 8.00000000,
   7.11111111,  6.40000000,  5.81818182,  5.33333333,  4.92307692,  4.57142857, 4.26666667, 4.00000000,
   3.76470588,  3.55555556,  3.36842105,  3.20000000,  3.04761905,  2.90909091, 2.78260870, 2.66666667,
   2.56000000,  2.46153846,  2.37037037,  2.28571429,  2.20689655,  2.13333333, 2.06451613, 2.00000000,
   1.93939394,  1.88235294,  1.82857143,  1.77777778,  1.72972973,  1.68421053, 1.64102564, 1.60000000,
   1.56097561,  1.52380952,  1.48837209,  1.45454545,  1.42222222,  1.39130435, 1.36170213, 1.33333333,
   1.30612245,  1.28000000,  1.25490196,  1.23076923,  1.20754717,  1.18518519, 1.16363636, 1.14285714,
   1.12280702,  1.10344828,  1.08474576,  1.06666667,  1.04918033,  1.03225806, 1.01587302, 1.00000000,
};
