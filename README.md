# paula-hdr
An expermiment for encoding 8-bit audio with dynamic gain for Paula.

## Theory of operation
The current defacto method of playing better than 8-bit samples via Paula on the Amiga uses a technique that takes 16-bit data and plays the most significant 8-bits in one channel at full volume and the next most significant 6 bits at minimum volume, thus attaining an effective 14-bit playback. This works because Paula has a per channel hardware volume control of 0 (silence) to 64 (maximum). Both channels are fed by the internal DMA engine which ensures that the component parts of each 16-bit sample are played at the same time.

The purpose of this code is to explore the possiblility of making Paula a better 8-bit player rather than a less than perfect 16-bit one.

### Paula Hardware
Paula uses 4 independent 8-bit DAC that in turn have a hardware volume control and have their own DMA, period and volume controls. The DAC themselves are known to be rather non-linear. This results in both adds to the characteristic sound of the chip and the need to perform a calibration to get the best playback quality.

In contrast, the volume control is rather linear. The volume control is realised via a PWM technique. The ~3.5Mhz clock signal to Paula is divided by 64 and each channel has a 6-bit cyclical counter that increments from 0 to 63 on each clock pulse. The DAC is enabled when the AUDxVOL (volume for channel X) register is above this value. Hence at 64, the DAC is always on and at 0, the DAC is always off. Every value in between results in the DAC being turned on and off for a given duty cycle.

For the maximum normal sample replay of ~28 kHz, this PWM completes two full cycles per input sample. It is assumed that the analogue side implements a reconstruction filter (or this happens as a natural consequence of some capacative load) that results in the time averaging of the PWM into an analogue singal level representitive of the hardware volume.

### Channel Modulation
Paula allows for some interesting hardware modulation. One channel can be used as a modulator for another, in which the modulator channel can control the period, volume or both of the carrier channel. Values read by the modulator are poked into the corresponding registers for the carrier. Since the volume control is entirely digital, it is assumed the effect is as near instantaeous.

Where a channel is used as a volume modulator, it reads a 16-bit value from the next position in the DMA buffer and applies it. The carrier channel will read two 8-bit samples in this time. This means that the maxumum modulation rate for volume is only half the maximum rate for sample playback. Since each channel has it's own period control, it is assumed that the modulator channel can be set to rates lower than this. For example, allowing the modulator to run at 1/4, 1/8, 1/16 etc. of the carrier rate.

### Compadiing
The above observations lead to the conclusion that it ought to be possible to define a format in which 16-bit source data is converted to 8-bit using a companding technique where the 16-bit data is scaled by some factor before conversion and that factor is then used to adjust the replay volume.

## Encoding
The encoder tool encodes a raw signed 16-bit mono file and cuts into frames of 16 samples. Each frame is scanned for the largest absolute value. This value is then used to determine an idealised normalizing factor for the frame. The closest normalizing factor that can be found based on having a linear 6-bit volume is chosen and the sample is then normalized and truncated to 8 bits.

An output frame is constructed comprising the expected Paula AUDxVOL value (range 1-64) and 16 8-bit signed values that are the truncated samples themselves. A special case is when the 14-bit frame is found to be silent, in that case we just record an AUDxVOL value of zero and don't include any sample values.

## Decoding
The decode operation tries to simulate how this would be replayed using Paula. The AUDxVOL value would be copied into 8 16-bit words and used for AM modulation of another channel which will be playing the corresponding 8-bit samples at the volume indicated.

## Real Paula Playback
This is still very much todo and it may prove that the hardware does not work exactly as intended in this scenario.
