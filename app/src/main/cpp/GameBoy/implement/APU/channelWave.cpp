#include "../../include/channelWave.h"

void channelWaveInit(ChannelWave* CHANNEL)
{ memset(CHANNEL, 0, sizeof(ChannelWave)); }

void channelWaveSetNR30(ChannelWave* CHANNEL, u8 VALUE)
{
  CHANNEL->NR30      = VALUE | 0x7F;
  CHANNEL->isEnabled = (VALUE & 0x80) != 0;
}

void channelWaveSetNR31(ChannelWave* CHANNEL, u8 VALUE)
{
  CHANNEL->NR31   = VALUE;
  CHANNEL->length = 256 - VALUE;
}

void channelWaveSetNR32(ChannelWave* CHANNEL, u8 VALUE)
{
  CHANNEL->NR32   = VALUE | 0x9F;
  CHANNEL->volume = (VALUE >> 5) & 0x03;
}

void channelWaveSetNRX3(ChannelWave* CHANNEL, u8 VALUE)
{ CHANNEL->periodLo = VALUE; }

void channelWaveSetNRX4(ChannelWave* CHANNEL, u8 VALUE)
{
  CHANNEL->NRX4         = VALUE | 0xBF;
  CHANNEL->trigger      = (VALUE & 0x80) != 0;
  CHANNEL->lengthEnable = (VALUE & 0x40) != 0;
  CHANNEL->periodHi     = (VALUE & 0x07);

  if (CHANNEL->trigger)
  {
    CHANNEL->trigger   = false;
    CHANNEL->isEnabled = true;

    if (CHANNEL->length == 0) CHANNEL->length = 256;

    CHANNEL->frequency = ((u16)CHANNEL->periodHi << 8) | CHANNEL->periodLo;
    CHANNEL->waveIndex = 0;
    CHANNEL->nibble    = 0;
  }
}

void channelWaveTickLength(ChannelWave* CHANNEL)
{
  if (CHANNEL->length > 0) CHANNEL->length--;
  if (CHANNEL->length == 0 && CHANNEL->lengthEnable) CHANNEL->isEnabled = false;
}

void channelWaveTickSampleGenerator(ChannelWave* CHANNEL, i32 CYCLES)
{
  CHANNEL->counter -= CYCLES;

  if (CHANNEL->counter <= 0)
  {
    CHANNEL->frequency = ((u16) CHANNEL->periodHi << 8) | CHANNEL->periodLo;
    CHANNEL->counter   = (2048 - CHANNEL->frequency) * 2;

    u8 waveByte = CHANNEL->wavePatternRAM[CHANNEL->waveIndex];
    u8 wave     = (waveByte >> (CHANNEL->nibble ? 0 : 4)) & 0xF;

    CHANNEL->nibble     ^= 1;
    if (CHANNEL->nibble == 0) CHANNEL->waveIndex   = (CHANNEL->waveIndex + 1) & 0x0F;

    if (CHANNEL->isEnabled)
    {
      u8 shift = 0;
      switch (CHANNEL->volume)
      {
        case 0 : { shift = 4; break; }
        case 1 : { shift = 0; break; }
        case 2 : { shift = 1; break; }
        case 3 : { shift = 2; break; }
      }

      CHANNEL->sample = wave >> shift;
    }
    else CHANNEL->sample = 0;
  }
}
