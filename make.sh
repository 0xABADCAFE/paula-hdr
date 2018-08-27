#!/bin/sh
gcc -Wall -W -O3 encoder.c -o bin/encoder
gcc -Wall -W -O3 decoder.c -o bin/decoder
gcc -Wall -W -O3 error.c -o bin/error -lm
#g++ -Wall -W -O3 encoder2.cpp -o bin/encoder2
