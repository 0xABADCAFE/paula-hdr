#ifndef __PAULA_HDR_ENCODER__
#define __PAULA_HDR_ENCODER__

#include "paulahdrfile.hpp"

class PaulaHDREncoder {

  private:
    static const float32 defaultVol[64];

    // Data buffers
    int16*    readPCMBuffer;
    int8*     writePCMBuffer;
    uint8*    writeVolBuffer;
    uint32    frameSize;
    uint32    blockSize;
    uint32    bufferSize;

    // Offsets into buffers
    uint32    readPCMBufferOffset;
    uint32    writePCMBufferOffset;
    uint32    writeVolBufferOffset;

  public:

    PaulaHDREncoder(uint32 frameSize, uint32 blockSize);
    ~PaulaHDREncoder();

    uint32 getFrameSize() const {
      return frameSize;
    }

    uint32 getBlockSize() const {
      return blockSize;
    }

    uint32 encode(PCMInput* input, PaulaHDRFileOutput* output);

  private:

    uint32 encodeBlock(PCMInput* input, PaulaHDRFileOutput* output);
    uint32 encodeFrame(uint32 length);

    void recordVolRunLength(uint32 lastRun) {
      writeVolBuffer[writeVolBufferOffset++] = 128|lastRun;
    }

};

#endif

