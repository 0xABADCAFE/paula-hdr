#include "include/types.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>
#include <cstring>

PaulaHDREncoder::PaulaHDREncoder(uint32 reqFrameSize, uint32 reqBlockSize) :
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

uint32 PaulaHDREncoder::encode(PCMInput* input, std::FILE* output) {

  if (!input/*|| !output*/) {
    std::fprintf(stderr, "No input stream available\n");
    return 0;
  }

  if (input->channels() != 1) {
    std::fprintf(stderr, "Only mono input data is supported\n");
    return 0;
  }

  input->start();
  uint32
    samplesRead = 0,
    lastRead    = 0,
    frameBlock  = 0;

  do {
    lastRead = encodeBlock(input, output);
    samplesRead += lastRead;

    std::fprintf(stderr, "Frameblock %u has %u entries...\n", frameBlock++, writeVolBuffer);

    for (uint32 i=0; i < writeVolBufferOffset; i++) {
      if (i&15 == 0) {
        std::fprintf(stderr, "\n\t");
      }
      std::fprintf(stderr, "%3u ", writeVolBuffer[i]);
    }
    std::fprintf(stderr, "\n");
  } while (lastRead == bufferSize);

  std::fprintf(stderr, "Total Samples: %u\n", samplesRead);

  return samplesRead;
}

uint32 PaulaHDREncoder::encodeBlock(PCMInput* input, std::FILE* output) {
  uint32 totSamples    = input->read(readPCMBuffer, bufferSize);
  if (totSamples > 0) {
    // Reset all our buffers
    readPCMBufferOffset  = 0;
    writePCMBufferOffset = 0;
    writeVolBufferOffset = 0;
    std::memset(writePCMBuffer, 0, bufferSize);
    std::memset(writeVolBuffer, 0, blockSize);

    uint32 numFrames  = totSamples / frameSize;
    uint32 lastVolume = 255;
    uint32 lastRun    = 0;
    for (uint32 i = 0; i < numFrames; i++, totSamples -= frameSize) {
      uint32 frameVolume = encodeFrame(frameSize);

      // If the frameVolume has changed, we need to cap any existing run length
      // and record the new valuie
      if (frameVolume != lastVolume) {

        // We were recording a runlength. Cap it off now.
        if (lastRun) {
          writeVolBuffer[writeVolBufferOffset++] = 128|lastRun;
          lastRun    = 0;
          lastVolume = frameVolume;
        }
        writeVolBuffer[writeVolBufferOffset++] = frameVolume;
      } else {
        lastRun++;

        // We can only store run values between upto 127, so if we exceed this, just
        // cap the current run with the existing volume.
        if (lastRun > 127) {
          writeVolBuffer[writeVolBufferOffset++] = 128|lastRun;
          writeVolBuffer[writeVolBufferOffset++] = lastVolume;
          lastRun    = 0;
        }
      }
    }

    // Handle a straggling short frame
    if (totSamples > 0) {
      uint32 frameVolume = encodeFrame(totSamples);
    }


  }
  return totSamples;
}

uint32 PaulaHDREncoder::encodeFrame(uint32 length) {

  int16* input    = readPCMBuffer  + readPCMBufferOffset;
  int8*  output   = writePCMBuffer + writePCMBufferOffset;
  uint32 num      = length;


  // Identify the largest absolute 14-bit value
  int16  max14bit = 0;
  while (num--) {
    int16 sample14bit = (*input++) >> 2;
    int16 abs14bit    = sample14bit < 0        ? -sample14bit : sample14bit;
    max14bit          = abs14bit    > max14bit ?  abs14bit    : max14bit;
  }

  if (max14bit) {
    float32 idealNormaliser = 8192.0 / (float32)max14bit;
    float32 bestNormaliser  = 0;
    int     normaliserIndex = 0;
    while (idealNormaliser < defaultVol[normaliserIndex]) {
      normaliserIndex++;
    }

    // The lookup table doesn't have an entry for zero (would be infinity) and we handle it
    // in a different way. Adding 1 onto the normaliserIndex effectively converts it into the
    // expected Paula AUDxVOL value we will need to replay the frame correctly.

    bestNormaliser = defaultVol[normaliserIndex++];
    int max8bit    = (int)(max14bit * bestNormaliser) >> 6;

    std::fprintf(
      stderr,
      "\tMax 14-bit:%4d (ideal %0.6f) AUDxVOL:%d (scale: %0.6f), 8-bit:%d, replay: %d\n",
      (int)max14bit,
      idealNormaliser,
      normaliserIndex,
      bestNormaliser,
      max8bit,
      (int)((max8bit << 6) / bestNormaliser)
    );

    // Reset the offsets for encoding the input
    num   = length;
    input = readPCMBuffer + readPCMBufferOffset;
    while (num--) {
      int scaled8bit = (int)((float32)(*input++ >> 2) * bestNormaliser) >> 6;

      // Perform some range clamping. Although -128 is valid, it results in underflow in decoding
      *output++ = scaled8bit < -127 ? -127 : scaled8bit > 127 ? 127 : scaled8bit;
    }

    // Move the buffer positions along
    readPCMBufferOffset  += length;
    writePCMBufferOffset += length;
    return normaliserIndex;

  }
  // Increment the read buffer position only as a silence frame doesn't use any output PCM space
  readPCMBufferOffset  += length;
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
