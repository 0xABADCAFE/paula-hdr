#!/bin/sh
bin/encoder data/test_raw_16bit_LE.raw data/encoded.raw > encode_log.txt
bin/decoder data/encoded.raw data/decoded.raw > decode_log.txt

play -r 44100 -b 16 -c 1 -e signed-integer data/test_raw_16bit_LE.raw
play -r 44100 -b 16 -c 1 -e signed-integer data/decoded.raw
