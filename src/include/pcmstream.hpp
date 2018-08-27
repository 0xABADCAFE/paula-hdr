#ifndef __PAULA_HDR_PCMSTREAM__
#define __PAULA_HDR_PCMSTREAM__

#include "types.hpp"
#include <cstdio>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PCMStream
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PCMStream {
  public:
    typedef enum {
      INT_8   = 1,
      INT_16  = 2,
    } Format;

    virtual Format  format() const   = 0;
    virtual uint32  channels() const = 0;
    virtual float64 rate() const     = 0;
    virtual bool    open(const char* source) = 0;
    virtual void    close() = 0;
    virtual         ~PCMStream() { }
};

class PCMInput : public PCMStream {
  public:
    virtual void start() = 0;
    virtual size_t read(void* destination, size_t count) = 0;
};

class PCMOutput : public PCMStream {
  public:
    virtual size_t write(size_t count, void* destination) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  RawStaticPCMInput
//
//  A pure template realisation of PCMInput for totally raw data with an assumed format.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<PCMStream::Format F, uint32 C, uint32 R> class RawStaticPCMInput : public PCMInput {

  private:
    static const size_t wordSize = F*C;
    std::FILE* stream;

  public:
    RawStaticPCMInput() : stream(0) { }

    void close() {
      if (stream) {
        std::fclose(stream);
        stream = 0;
        std::printf("Closed stream\n");
      }
    }

    bool open(const char* source) {
      close();
      stream = std::fopen(source, "rb");
      if (stream) {
        std::printf("Opened stream %s\n", source);
      } else {
        std::printf("Failed opening stream %s\n", source);
      }
      return stream != 0;
    }

    Format  format() const   { return F; }
    uint32  channels() const { return C; }
    float64 rate() const     { return R; }

    void start() {
      if (stream) {
        std::fseek(stream, 0, SEEK_SET);
      }
    }

    size_t read(void* destination, size_t count) {
      if (stream) {
        return std::fread(destination, wordSize, count, stream);
      }
      return 0;
    }

    ~RawStaticPCMInput() {
      close();
    }
};

#endif

