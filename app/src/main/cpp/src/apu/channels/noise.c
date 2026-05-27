#include <apu/channels/noise.h>
#include <apu/internal.h>

void noiseTrigger(NoiseChannel* CHANNEL) 
{
  CHANNEL->enabled = true;
  
  if (CHANNEL->lengthTimer == 0) CHANNEL->lengthTimer = CH_NOISE_LENGTH_MAX;
  
  // - - - Reset LFSR to all 1s
  CHANNEL->lfsr = LFSR_INITIAL_VALUE;
  
  CHANNEL->envelopeTimer = CHANNEL->envelopePace;
  CHANNEL->currentVolume = CHANNEL->initialVolume;

  u32 divisor = (CHANNEL->clockDivider == 0) ? 8 : (CHANNEL->clockDivider << 4);
  CHANNEL->periodTimer = (divisor << CHANNEL->clockShift) >> 2;
  
  if (!CHANNEL->dacEnabled) CHANNEL->enabled = false;
}

void noiseClockLength(NoiseChannel* CHANNEL) 
{
  if (CHANNEL->lengthEnabled && CHANNEL->lengthTimer > 0) 
  {
    CHANNEL->lengthTimer--;
    if (CHANNEL->lengthTimer == 0) CHANNEL->enabled = false;
  }
}

void noiseClockEnvelope(NoiseChannel* CHANNEL) 
{
  if (CHANNEL->envelopePace != 0) 
  {
    if (CHANNEL->envelopeTimer > 0) CHANNEL->envelopeTimer--;
    
    if (CHANNEL->envelopeTimer == 0) 
    {
      CHANNEL->envelopeTimer = CHANNEL->envelopePace;
        
      if (CHANNEL->envelopeIncrease && CHANNEL->currentVolume < CH_MAX_VOLUME) 
      { CHANNEL->currentVolume++; } 
      else if (!CHANNEL->envelopeIncrease && CHANNEL->currentVolume > 0) 
      { CHANNEL->currentVolume--; }
    }
  }
}

void noiseStepTimer(NoiseChannel* CHANNEL) 
{
  CHANNEL->outputVolume = 0;

  if (!CHANNEL->enabled || !CHANNEL->dacEnabled)  return;

  // THE FIX: Using a clock shift of 14 or 15 completely stops the LFSR from receiving clocks
  if (CHANNEL->clockShift >= 14) return;

  if (CHANNEL->periodTimer > 0)                   CHANNEL->periodTimer--;
  
  if (CHANNEL->periodTimer == 0) 
  {
    // THE FIX: Calculate Period Timer based on hardware formula. 
    // The standard formula (Divisor = 8 or r*16) is in T-Cycles (4.19 MHz).
    // Because our APU ticks in M-Cycles (1.04 MHz), we must divide the final period by 4!
    u32 divisor = (CHANNEL->clockDivider == 0) ? 8 : (CHANNEL->clockDivider << 4);
    
    // Shift right by 2 divides the T-Cycle period by 4, perfectly syncing it to our M-Cycle loop
    CHANNEL->periodTimer = (divisor << CHANNEL->clockShift) >> 2;

    // - - - Advance the LFSR
    u8 xorBit = (CHANNEL->lfsr & 0x01) ^ ((CHANNEL->lfsr & 0x02) >> 1);
    CHANNEL->lfsr >>= 1;
    CHANNEL->lfsr |= (xorBit << 14);

    // - - - If in short mode (7-bit), duplicate the XOR bit to bit 6
    if (CHANNEL->shortMode) 
    {
      CHANNEL->lfsr &= ~(1 << 6); // Clear bit 6
      CHANNEL->lfsr |= (xorBit << 6);
    }
  }

  // - - - Emit Volume (Inverted: output volume if bit 0 is 0)
  if ((CHANNEL->lfsr & 0x01) == 0) 
  { CHANNEL->outputVolume = CHANNEL->currentVolume; }
}
