#include <stdio.h>
#include <math.h>
#include "types.h"

#define BUFFERSIZE 256

int main(int argc, const char** argv) {
  if (argc > 2) {
    FILE* a = fopen(argv[1], "rb");

    if (a) {
      FILE* b = fopen(argv[2], "rb");

      if (b) {
        int16   abuffer[BUFFERSIZE];
        int16   bbuffer[BUFFERSIZE];
        size_t  num   = 0;
        float64 error = 0.0;
        size_t  count;

        printf("Comparing %s and %s...\n", argv[1], argv[2]);

        do {
          size_t aread = fread(abuffer, sizeof(int16), BUFFERSIZE, a);
          size_t bread = fread(bbuffer, sizeof(int16), BUFFERSIZE, b);
          count        = aread > bread ? bread : aread;

          for (size_t i = 0; i < count; i++) {
            float64 diff = (float64)bbuffer[i] - (float64)abuffer[i];
            error += diff * diff;
          }
          num += count;
        } while (count == BUFFERSIZE);

        fclose(b);

        error = sqrt(error / (float64)num);

        printf("Total RMS error %.2f based on %d samples compared.\n", error, (int)num);

      } else {
        printf("Failed to open %s\n", argv[2]);
      }

      fclose(a);

    } else {
      printf("Failed to open %s\n", argv[1]);
    }

  } else {
    puts("Estimate the RMS error between the original and decoded output files.\n\tUsage encode <raw file 1> <raw file 2>\n");
  }
}

