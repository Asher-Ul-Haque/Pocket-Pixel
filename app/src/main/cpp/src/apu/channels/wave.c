#include <apu/channels/wave.h>

void waveTrigger(WaveChannel* CHANNEL) 
{
  CHANNEL->enabled = true;

  if (CHANNEL->lengthTimer == 0) CHANNEL->lengthTimer = CH_WAVE_LENGTH_MAX;

  CHANNEL->periodTimer   = APU_PERIOD_MAX_VALUE - CHANNEL->periodValue;
  CHANNEL->positionIndex = 0; // - - - Triggering resets the playhead

  if (!CHANNEL->dacEnabled) CHANNEL->enabled = false;
}

void waveClockLength(WaveChannel* CHANNEL)
{
  if (CHANNEL->lengthEnabled && CHANNEL->lengthTimer > 0) 
  {
    CHANNEL->lengthTimer--;
    if (CHANNEL->lengthTimer == 0) CHANNEL->enabled = false;
  }
}

void waveStepTimer(WaveChannel* CHANNEL)
{
  CHANNEL->outputVolume = 0;
  if (!CHANNEL->enabled || !CHANNEL->dacEnabled) return;

  // - - - The Wave channel runs at 2MHz (double the 1MHz M-Cycle speed).
  for (u8 i = 0; i < WAVE_TIMER_SPEED_MULTI; i++) 
  {
    CHANNEL->periodTimer--;
      
    if (CHANNEL->periodTimer == 0) 
    {
       CHANNEL->periodTimer   = APU_PERIOD_MAX_VALUE - CHANNEL->periodValue;
       CHANNEL->positionIndex = (CHANNEL->positionIndex + 1) % WAVE_SAMPLE_COUNT;
    }
  }

  // - - - Extract the active 4-bit sample from the 16-byte array
  u8 byteIndex = CHANNEL->positionIndex / 2;
  u8 byteValue = CHANNEL->waveRam[byteIndex];
  
  // - - - Even indices are the upper nibble, odd indices are the lower nibble
  u8 sample = (CHANNEL->positionIndex % 2 == 0) ? (byteValue >> 4) : (byteValue & 0x0F);

  // - - - Apply the hardware volume shift
  switch (CHANNEL->volumeCode) 
  {
    case 0: CHANNEL->outputVolume = 0;            break; ///< Mute
    case 1: CHANNEL->outputVolume = sample;       break; ///< 100% volume
    case 2: CHANNEL->outputVolume = sample >> 1;  break; ///< 50% volume
    case 3: CHANNEL->outputVolume = sample >> 2;  break; ///< 25% volume
  }
}

u8 waveReadRam(WaveChannel* CHANNEL, u16 ADDR) 
{
  u8 index = ADDR - REG_WAVE_RAM_START;
  return CHANNEL->waveRam[index];
}

void waveWriteRam(WaveChannel* CHANNEL, u16 ADDR, u8 VALUE) 
{
  u8 index = ADDR - REG_WAVE_RAM_START;
  CHANNEL->waveRam[index] = VALUE;
}
