#ifndef __PAULA_HDR_FILE___
#define __PAULA_HDR_FILE__

#include <cstdio>

class PaulaHDRFile {
  protected:
    struct Header {
      char   magic[4];
      uint8  sampleRateMSB;
      uint8  sampleRateLSB;
      uint8  blockSize;
      uint8  frameSize;
    } header;

  public:
    enum {
      MIN_FRAMESIZE = 8,
      MAX_FRAMESIZE = 256,
      MIN_BLOCKSIZE = 1,
      MAX_BLOCKSIZE = 128
    };

    uint32  getSampleRate() const {
      return (uint32)header.sampleRateMSB << 8 | header.sampleRateLSB;
    }

    uint32  getBlockSize() const {
      return header.blockSize;
    }

    uint32  getFrameSize() const {
      return 1 + (uint32)header.frameSize;
    }

    virtual bool open(const char* source) = 0;
    virtual void close() = 0;

    virtual ~PaulaHDRFile() { };
};

class PaulaHDRFileOutput : public PaulaHDRFile {

  private:
    std::FILE* stream;

  public:
    PaulaHDRFileOutput();

    void   setSampleRate(uint16 rate);
    bool   setBlockSize(uint32 blockSize);
    bool   setFrameSize(uint32 frameSize);
    size_t writeBlock(uint8* blockData, int8* frameData, uint16 blockDataSize, uint16 frameDataSize);
    bool   open(const char* destination);
    void   close();

    ~PaulaHDRFileOutput();
};

#endif

