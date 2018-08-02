# paula-hdr
An expermiment for encoding 8-bit audio with dynamic gain for Paula. The encoder tool encodes a raw signed 16-bit mono file and cuts into frames of 16 samples. Each frame is then converted to 14 bit, scanned for the largest absolute value. This value is then used to determine an idealised normalizing factor for the frame. The closest normalizing factor that can be found based on having a linear 6-bit volume is chosen and the sample is then normalized and truncated to 8 bits.
An output frame is constructed comprising the expected Paula AUDxVOL value (range 1-64) and 16 8-bit signed values that are the truncated samples themselves. A special case is when the 14-bit frame is found to be silent, in that case we just record an AUDxVOL value of zero and don't include any sample values.
e
The decode operation tries to simulate how this would be replayed using Paula. The AUDxVOL value would be copied into 8 16-bit words and used for AM modulation of another channel which will be playing the corresponding 8-bit samples at the volume indicated.

There are a few clipping bugs to sort out.
