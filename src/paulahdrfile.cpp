#include "include/types.hpp"
#include "include/paulahdrfile.hpp"

uint32 PaulaHDRFile::clampFrameSize(uint32 frameSize) {
  frameSize = (frameSize & ~1);
  return frameSize < MIN_FRAMESIZE ? MIN_FRAMESIZE : frameSize > MAX_FRAMESIZE ? MAX_FRAMESIZE : frameSize;
}

uint32 PaulaHDRFile::clampBlockSize(uint32 blockSize) {
  return blockSize < MIN_BLOCKSIZE ? MIN_BLOCKSIZE : blockSize > MAX_BLOCKSIZE ? MAX_BLOCKSIZE : blockSize;
}

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

void PaulaHDRFileOutput::setBlockSize(uint32 blockSize) {
  header.blockSize = (uint8) clampBlockSize(blockSize);
}

void PaulaHDRFileOutput::setFrameSize(uint32 frameSize) {
  header.frameSize = (uint8) (clampFrameSize(frameSize) - 1);
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
