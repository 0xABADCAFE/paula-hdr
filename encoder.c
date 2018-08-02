#include <stdio.h>

#include "types.h"
#include "encoder_tables.h"

size_t encode_frame(const int16* input, int8* output, size_t framelen) {
  const int16* source   = input;
  size_t       num      = framelen;
  int16        max14bit = 0;
  while (num--) {
    int16 sample14bit = (*source++) >> 2;
    int16 abs14bit    = sample14bit < 0        ? -sample14bit : sample14bit;
    max14bit          = abs14bit    > max14bit ?  abs14bit    : max14bit;
    max14bit          = max14bit    > 8191     ?  8191        : max14bit;
  }

  if (max14bit) {
    float32 ideal_factor = 8191.0 / (float32)max14bit;
    float32 best_factor  = 0;
    int     index        = 0;
    while (ideal_factor < amp_factors[index]) {
      index++;
    }

    best_factor = amp_factors[index];

    int sample8 = (int)(max14bit * best_factor) >> 6;

    printf(
      "\tMax 14-bit:%4d (ideal %0.6f) AUDxVOL:%d (scale: %0.6f), 8-bit:%d, replay: %d\n",
      (int)max14bit,
      ideal_factor,
      index + 1,
      best_factor,
      sample8,
      (int)((sample8 << 6) / best_factor)
    );

    source    = input;
    num       = framelen;
    *output++ = index + 1;
    while (num--) {
      *output++ = ((int)((float32)(*source++ >> 2) * best_factor) >> 6);
    }

    return framelen+1;
  } else {
    puts("\tEncoded silent frame.");
    *output++ = 0;
    return 1;
  }
}

int main(int argc, const char** argv) {

  const int FRAME_SIZE = 16;

  if (argc > 2) {
    FILE* input = fopen(argv[1], "rb");
    if (input) {
      FILE* output = fopen(argv[2], "wb");
      if (output) {
        size_t samples_read = 0;
        int    frame_number = 0;
        int16  input_frame[FRAME_SIZE];
        int8   output_frame[FRAME_SIZE+1];

        do {
          samples_read = fread(input_frame, sizeof(int16), FRAME_SIZE, input);
          if (samples_read > 0) {
            size_t encoded_size;
            printf("Read frame %d [%d] - ", frame_number++, (int)samples_read);
            encoded_size = encode_frame(input_frame, output_frame, samples_read);
            fwrite(output_frame, sizeof(int8), encoded_size, output);
          }
        } while (samples_read == FRAME_SIZE);
        fclose(output);
      }
      fclose(input);
    }
  }

  return 0;
}

