#ifndef __PAULA_HDR_ENCODER__
#define __PAULA_HDR_ENCODER__

class PaulaHDREncoder {

  private:
    static const float32 defaultVol[64];

    int16*    readPCMBuffer;
    int8*     writePCMBuffer;
    uint8*    writeVolBuffer;
    size_t    frameSize;
    size_t    blockSize;
    size_t    bufferSize;
  public:
    enum {
      MIN_FRAMESIZE = 8,
      MAX_FRAMESIZE = 256,
      MIN_BLOCKSIZE = 1,
      MAX_BLOCKSIZE = 128
    };

    PaulaHDREncoder(size_t frameSize, size_t blockSize);
    ~PaulaHDREncoder();

    size_t getFrameSize() const {
      return frameSize;
    }

    size_t getBlockSize() const {
      return blockSize;
    }

    size_t encode(PCMInput* input, std::FILE* output);

};

#endif
