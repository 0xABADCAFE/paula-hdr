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

    const char* get(const char* name, const char* def = 0) const {
      int argMax = argCount - 1;
      for (int i = 0; i < argMax; i++) {
        if (i < argCount && !strcmp(name, argList[i])) {
          return argList[i + 1];
        }
      }
      return def;
    }

    bool has(const char* name) const {
      for (int i = 0; i < argCount; i++) {
        if (!strcmp(name, argList[i])) {
          return true;
        }
      }
      return false;
    }
};

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
    virtual         ~PCMStream()     = 0;
    virtual Format  format() const   = 0;
    virtual uint32  channels() const = 0;
    virtual float64 rate() const     = 0;
};

class PCMInput : public PCMStream {
  public:
    virtual size_t read(void* destination, size_t count) = 0;
};

class PCMOutput : public PCMStream {
  public:
    virtual size_t write(size_t count, void* destination) = 0;
};

template<PCMStream::Format F, uint32 C, uint32 R> class RawPCMInput : public PCMInput {

  private:
    static const size_t wordSize = F*C;
    std::FILE* stream;

  public:
    ~RawPCMInput() { }

    Format  format() const   { return F; }
    uint32  channels() const { return C; }
    float64 rate() const     { return R; }

    size_t read(void* destination, size_t count) {
      return 0;
    }
};

typedef RawPCMInput<PCMStream::INT_16, 1, 44100> PCMSource;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char **argv) {

  CLIParameters params(argc, argv);

  const char* from = params.get("-f");
  const char* to   = params.get("-t");

  if (from && to) {
    printf("From: %s To: %s\n", from, to);
  }
  return 0;
}
