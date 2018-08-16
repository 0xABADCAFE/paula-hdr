#include "include/types.hpp"
#include "include/pcmstream.hpp"
#include "include/encoder.hpp"
#include <cstdio>
#include <cstring>

/**
 * Constructor
 */
PaulaHDREncoder::PaulaHDREncoder(uint32 reqFrameSize, uint32 reqBlockSize, bool verbose) :
  readPCMBuffer(0),
  writePCMBuffer(0),
  writeVolBuffer(0),
  verbose(verbose)
{
  frameSize  = PaulaHDRFile::clampFrameSize(reqFrameSize);
  blockSize  = PaulaHDRFile::clampBlockSize(reqBlockSize);
  bufferSize = frameSize * blockSize;

  if (verbose) {
    std::fprintf(
      stderr,
      "Frame Length %d, Block Length %d, BufferLength %d\n",
      (int)frameSize,
      (int)blockSize,
      (int)bufferSize
    );
  }
  readPCMBuffer  = new int16[bufferSize];
  writePCMBuffer = new int8[bufferSize];
  writeVolBuffer = new uint8[blockSize];
}

/**
 * Destructor
 */
PaulaHDREncoder::~PaulaHDREncoder() {
  delete[] readPCMBuffer;
  delete[] writePCMBuffer;
  delete[] writeVolBuffer;
}

/**
 * Stream encode
 */
uint32 PaulaHDREncoder::encode(PCMInput* input, PaulaHDRFileOutput* output) {

  if (!input) {
    if (verbose) {
      std::fprintf(stderr, "No input stream available\n");
    }
    return 0;
  }

  if (!output) {
    if (verbose) {
      std::fprintf(stderr, "No output stream available\n");
    }
    return 0;
  }

  if (input->channels() != 1) {
    if (verbose) {
      std::fprintf(stderr, "Only mono input data is supported\n");
    }
    return 0;
  }

  input->start();
  uint32
    samplesRead = 0,
    lastRead    = 0,
    frameBlock  = 0;

  do {
    if (verbose) {
      std::fprintf(stderr, "\nEncoding Frameblock %u...\n", frameBlock++);
    }

    lastRead = encodeBlock(input);
    samplesRead += lastRead;

    if (verbose) {
      std::fprintf(stderr, "\tTotal %u entries for %u samples...\n\t", writeVolBufferOffset, lastRead);
      for (uint32 i=0; i < writeVolBufferOffset; i++) {
        std::fprintf(stderr, "%3u ", writeVolBuffer[i]);
      }
      std::fprintf(stderr, "\n");
    }

    deltaEncodeBuffer();

    output->writeBlock(
      writeVolBuffer,
      writePCMBuffer,
      writeVolBufferOffset,
      writePCMBufferOffset
    );

  } while (lastRead == bufferSize);
  if (verbose) {
    std::fprintf(stderr, "\nTotal Samples: %u\n", samplesRead);
  }
  return samplesRead;
}

/**
 * Encode an input block. This will read frameSize * blockSize samples and analyse the peak volume for each frame.
 * The volume data is RLE encoded.
 */
uint32 PaulaHDREncoder::encodeBlock(PCMInput* input) {
  uint32 totSamples = input->read(readPCMBuffer, bufferSize);
  uint32 result     = totSamples;
  if (totSamples > 0) {
    if (verbose) {
      std::fprintf(
        stderr,
        "\tPeak 14-bit | Ideal Scale | Volume | Actual Scale | Peak 8-bit | Replay 14-bit\n"
      );
    }
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
          recordVolRunLength(lastRun);
          lastRun    = 0;
        }
        writeVolBuffer[writeVolBufferOffset++] = frameVolume;
        lastVolume = frameVolume;
      } else {
        lastRun++;

        // We can only store run values between upto 127, so if we exceed this, just
        // cap the current run with the existing volume.
        if (lastRun > 127) {
          recordVolRunLength(lastRun);
          writeVolBuffer[writeVolBufferOffset++] = lastVolume;
          lastRun    = 0;
        }
      }
    }

    if (lastRun) {
      recordVolRunLength(lastRun);
    }
  }
  return result;
}

/**
 * Encode a single frame of data.
 */
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
    if (verbose) {
      int max8bit    = (int)(max14bit * bestNormaliser) >> 6;

      std::fprintf(
        stderr,
        "\t%11d | %11.6f | %6d |  %11.6f | %10d | %13d\n",
        (int)max14bit,
        idealNormaliser,
        normaliserIndex,
        bestNormaliser,
        max8bit,
        (int)((max8bit << 6) / bestNormaliser)
      );
    }
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

/**
 * Transform the output 8-bit PCM buffer into a set delta values. We may choose to encode these.
 */
void PaulaHDREncoder::deltaEncodeBuffer() {
  int8*  buffer   = writePCMBuffer;
  int8   last     = 0;
  uint32 length   = writePCMBufferOffset;
  while (length--) {
    int8 current = *buffer;
    *buffer++ = current - last;
    last = current;
  }
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
