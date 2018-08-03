#include <stdio.h>

#include "types.h"
#include "encoder_tables.h"

/**
 * Encode a frame of 16-bit audio to AM 8-bit format.
 * The input is a frame of N 16-bit signed PCM samples. The output is a frame of N-1 signed 8-bit bytes where the first
 * byte encodes the 6-bit AM channel volume and the remaining N bytes encode the gain-adjusted 8-bit versions of the the
 * input.
 *
 * The process treats the 16-bit source data as 14-bit replayable data by discarding the lowest 2 bits. The largest
 * 14-bit value is found and that is then used to work out the ideal Paula channel volume for this frame and what to
 * multiply each sample by before converting to 8-bit.
 */
size_t encode_frame(const int16* input, int8* output, size_t framelen) {
  const int16* source   = input;
  size_t       num      = framelen;
  int16        max14bit = 0;

  /* Identify the loudest 14-bit sample value */
  while (num--) {
    int16 sample14bit = (*source++) >> 2;
    int16 abs14bit    = sample14bit < 0        ? -sample14bit : sample14bit;
    max14bit          = abs14bit    > max14bit ?  abs14bit    : max14bit;
  }

  /* If we comnputed a non-zero maximum 14-bit sample, work out the best AM volume to use and convert the frame */
  if (max14bit) {
    float32 ideal_factor = 8192.0 / (float32)max14bit;
    float32 best_factor  = 0;
    int     index        = 0;
    while (ideal_factor < amp_factors[index]) {
      index++;
    }

    best_factor = amp_factors[index];
    int max8bit = (int)(max14bit * best_factor) >> 6;

    printf(
      "\tMax 14-bit:%4d (ideal %0.6f) AUDxVOL:%d (scale: %0.6f), 8-bit:%d, replay: %d\n",
      (int)max14bit,
      ideal_factor,
      index + 1,
      best_factor,
      max8bit,
      (int)((max8bit << 6) / best_factor)
    );

    source    = input;
    num       = framelen;

    /* Write the AM volume value. This is in the range 1-64 for Paula */
    *output++ = index + 1;

    /* Compute and write the maximised 8-bit samples */
    while (num--) {
      /* Calculate the next AM maximised 8-bit sample value */
      int val = (int)((float32)(*source++ >> 2) * best_factor) >> 6;

      /* Perform some range clamping. Although -128 is valid, it results in underflow in decoding */
      *output++ = val < -127 ? -127 : val > 127 ? 127 : val;
    }

    return framelen + 1;
  } else {
    /* If we got a zero maximum 14-bit sample, record a single zero byte for the whole silent frame */
    puts("\tEncoded silent frame.");
    *output++ = 0;
    return 1;
  }
}

int main(int argc, const char** argv) {

  const size_t FRAME_SIZE = 16;

  if (argc > 2) {
    FILE* input = fopen(argv[1], "rb");

    if (input) {
      FILE* output = fopen(argv[2], "wb");

      if (output) {
        size_t samples_read = 0;
        int    frame_number = 0;
        int16  input_frame[FRAME_SIZE];
        int8   output_frame[FRAME_SIZE + 1];

        do {
          samples_read = fread(input_frame, sizeof(int16), FRAME_SIZE, input);
          if (samples_read > 0) {
            size_t encoded_size = encode_frame(input_frame, output_frame, samples_read);
            printf("Read frame %d [%d] - ", frame_number++, (int)samples_read);
            fwrite(output_frame, sizeof(int8), encoded_size, output);
          }
        } while (samples_read == FRAME_SIZE);

        fclose(output);

      } else {
        printf("Couldn't open encoded output file %s\n", argv[2]);
      }

      fclose(input);

    } else {
      printf("Couldn't open raw input file %s\n", argv[1]);
    }

  } else {
    puts("Encode a raw 16-bit mono audio to AM8 format.\n\tUsage encode <raw 16-bit mono source> <output file>\n");
  }

  return 0;
}

