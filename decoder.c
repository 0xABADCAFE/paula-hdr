#include <stdio.h>
#include <string.h>
#include "types.h"

int main(int argc, const char** argv) {

  const int FRAME_SIZE = 16;

  if (argc > 2) {
    FILE* input = fopen(argv[1], "rb");
    if (input) {
      FILE* output = fopen(argv[2], "wb");
      int frame = 0;
      if (output) {
        size_t bytes_read;
        int16  output_frame[FRAME_SIZE];
        int8   input_frame[FRAME_SIZE];
        printf("Decoding %s to %s ...\n", argv[1], argv[2]);
        do {
          int volume_index = fgetc(input);
          bytes_read = 1;
          if (volume_index == EOF) {
             puts("End of input");
             break;
          } else if (volume_index == 0) {
            // Silence is golden
            memset(output_frame, 0, sizeof(output_frame));
            bytes_read = sizeof(input_frame);
            printf("Frame %d AUDxVOL 0\n", frame);
          } else {
            float32 attenuation = volume_index * 0.015626f; // 1/64.0
            int i;
            bytes_read = fread(input_frame, sizeof(int8), FRAME_SIZE, input);
            printf("Frame %d AUDxVOL %d: %.7f\n", frame, volume_index, attenuation);

            // Simulate the effect of playing the 8-bit normalised frame at the attenuation level
            // implied by the AUDxVOL value.
            for (i = 0; i < FRAME_SIZE; i++) {
               int sample = (int)input_frame[i] << 8;
               output_frame[i] = (int16) (sample * attenuation);
            }
          }

          fwrite(output_frame, sizeof(int16), FRAME_SIZE, output);
          frame++;
        } while (bytes_read == FRAME_SIZE);
        fclose(output);
      }
      fclose(input);
    }
  }

  return 0;
}

