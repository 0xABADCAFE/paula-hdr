#ifndef __PAULA_HDR_ENCODER__
#define __PAULA_HDR_ENCODER__

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
    enum {
      MIN_FRAMESIZE = 8,
      MAX_FRAMESIZE = 256,
      MIN_BLOCKSIZE = 1,
      MAX_BLOCKSIZE = 128
    };

    PaulaHDREncoder(uint32 frameSize, uint32 blockSize);
    ~PaulaHDREncoder();

    uint32 getFrameSize() const {
      return frameSize;
    }

    uint32 getBlockSize() const {
      return blockSize;
    }

    uint32 encode(PCMInput* input, std::FILE* output);
    uint32 encodeBlock(PCMInput* input, std::FILE* output);
    uint32 encodeFrame(uint32 length);
};

#endif

