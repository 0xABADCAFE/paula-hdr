#include "include/types.hpp"
#include "include/paulahdrfile.hpp"

PaulaHDRFileOutput::PaulaHDRFileOutput() : stream(0) {
  header.magic[0]      = 'P';
  header.magic[1]      = 'H';
  header.magic[2]      = 'D';
  header.magic[3]      = 'R';
  header.sampleRateMSB = 0;
  header.sampleRateLSB = 0;
  header.blockSize     = 0;
  header.frameSize     = 0;
}

void PaulaHDRFileOutput::setSampleRate(uint16 rate) {
  header.sampleRateMSB = (uint8)(rate >> 8);
  header.sampleRateLSB = (uint8)(rate & 0xFF);
}

bool PaulaHDRFileOutput::setBlockSize(uint32 blockSize) {
  if (blockSize < MIN_BLOCKSIZE || blockSize > MAX_BLOCKSIZE) {
    return false;
  }
  header.blockSize = (uint8)blockSize;
  return true;
}

bool PaulaHDRFileOutput::setFrameSize(uint32 frameSize) {
  if (frameSize < MIN_FRAMESIZE || frameSize > MAX_FRAMESIZE) {
    return false;
  }
  header.frameSize = (uint8) (frameSize - 1);
  return true;
}

size_t PaulaHDRFileOutput::writeBlock(uint8* blockData, int8* frameData, uint16 blockDataSize, uint16 frameDataSize) {
  return std::fwrite(blockData, 1, blockDataSize, stream) + std::fwrite(frameData, 1, frameDataSize, stream);
}

bool PaulaHDRFileOutput::open(const char* destination) {
  if (stream) {
    return true;
  }

  stream = std::fopen(destination, "wb");
  if (!stream) {
    return false;
  }
  size_t written = std::fwrite(&header, 1, sizeof(Header), stream);
  if (written != sizeof(Header)) {
    close();
    return false;
  }
  return true;
}

void PaulaHDRFileOutput::close() {
  if (stream) {
    std::fclose(stream);
    stream = 0;
  }
}

PaulaHDRFileOutput::~PaulaHDRFileOutput() {
  close();
}
