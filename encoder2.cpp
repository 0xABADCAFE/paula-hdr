#include "types.h"
#include <cstring>
#include <cstdio>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CLIParameters
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLIParameters {
  private:
    int          argCount;
    const char** argList;

  public:
    CLIParameters(int argCount, const char** argList) : argCount(argCount - 1), argList(argList + 1) { }

    bool has(const char* name) const;
    const char* get(const char* name, const char* def = 0) const ;
};

const char* CLIParameters::get(const char* name, const char* def) const {
  int argMax = argCount - 1;
  for (int i = 0; i < argMax; i++) {
    if (i < argCount && !strcmp(name, argList[i])) {
      return argList[i + 1];
    }
  }
  return def;
}

bool CLIParameters::has(const char* name) const {
  for (int i = 0; i < argCount; i++) {
    if (!strcmp(name, argList[i])) {
      return true;
    }
  }
  return false;
}

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
    virtual         ~PCMStream() { }
    virtual Format  format() const   = 0;
    virtual uint32  channels() const = 0;
    virtual float64 rate() const     = 0;
    virtual bool    open(const char* source) = 0;
    virtual void    close() = 0;
};

class PCMInput : public PCMStream {
  public:
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
//  A pure template realisation of PCMInput for totally raw data with an assumed format,
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<PCMStream::Format F, uint32 C, uint32 R> class RawStaticPCMInput : public PCMInput {

  private:
    static const size_t wordSize = F*C;
    std::FILE* stream;

  public:
    RawStaticPCMInput() : stream(0) {
    }

    ~RawStaticPCMInput() {
      close();
    }

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

    size_t read(void* destination, size_t count) {
      if (stream) {
        return std::fread(destination, wordSize, count, stream);
      }
      return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



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
