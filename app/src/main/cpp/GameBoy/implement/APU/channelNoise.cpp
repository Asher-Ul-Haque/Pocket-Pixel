#include "../../include/channelNoise.h"

void channelNoiseInit(ChannelNoise* CHANNEL)
{
  FORGE_ASSERT_MESSAGE(CHANNEL, "Cannot init null channel");

  CHANNEL->NR41 = 0;
  CHANNEL->NR42 = 0;
  CHANNEL->NR43 = 0;
  CHANNEL->NR44 = 0;

  CHANNEL->length = 0;
  CHANNEL->envelopeInitialVolume = 0;
  CHANNEL->envelopeDirection = 0;
  CHANNEL->envelopeSweep = 0;
  CHANNEL->envelopeCounter = 0;
  CHANNEL->envelopeVolume = 0;

  CHANNEL->clockShift = 0;
  CHANNEL->lfsrWidth = 0;
  CHANNEL->clockDivider = 0;
  CHANNEL->period = 0;
  CHANNEL->counter = 0;

  CHANNEL->lfsr = 0x7FFF;

  CHANNEL->isEnabled = false;
  CHANNEL->trigger = false;
  CHANNEL->lengthEnable = false;

  CHANNEL->sample = 0;
}

void channelNoiseSetNR41(ChannelNoise* CHANNEL, u8 VALUE)
{
  CHANNEL->NR41 = VALUE | 0xC0;
  CHANNEL->length = 64 - (VALUE & 0x3F);
}

void channelNoiseSetNR42(ChannelNoise* CHANNEL, u8 VALUE)
{
  CHANNEL->NR42 = VALUE;
  CHANNEL->envelopeInitialVolume = (VALUE >> 4) & 0xF;
  CHANNEL->envelopeDirection = (VALUE >> 3) & 0x1;
  CHANNEL->envelopeSweep = VALUE & 0x7;
}

void channelNoiseSetNR43(ChannelNoise* CHANNEL, u8 VALUE)
{
  CHANNEL->NR43 = VALUE;
  CHANNEL->clockShift = (VALUE >> 4) & 0xF;
  CHANNEL->lfsrWidth = (VALUE >> 3) & 0x1;
  CHANNEL->clockDivider = VALUE & 0x7;

  int divisor = 0;
  switch (CHANNEL->clockDivider)
  {
    case 0: divisor = 8; break;
    case 1: divisor = 16; break;
    case 2: divisor = 32; break;
    case 3: divisor = 48; break;
    case 4: divisor = 64; break;
    case 5: divisor = 80; break;
    case 6: divisor = 96; break;
    case 7: divisor = 112; break;
  }

  CHANNEL->period = divisor << CHANNEL->clockShift;
}

void channelNoiseSetNR44(ChannelNoise* CHANNEL, u8 VALUE)
{
  CHANNEL->NR44 = VALUE | 0xBF;
  CHANNEL->trigger = (VALUE & 0x80) != 0;
  CHANNEL->lengthEnable = (VALUE & 0x40) != 0;

  if (CHANNEL->trigger)
  {
    CHANNEL->trigger = false;
    CHANNEL->isEnabled = true;
    if (CHANNEL->length == 0) CHANNEL->length = 64;

    CHANNEL->envelopeVolume = CHANNEL->envelopeInitialVolume;
    CHANNEL->envelopeCounter = CHANNEL->envelopeSweep;

    CHANNEL->counter = CHANNEL->period;
    CHANNEL->lfsr = 0x7FFF;
  }
}

void channelNoiseTickLength(ChannelNoise* CHANNEL)
{
  if (CHANNEL->length > 0) CHANNEL->length--;
  if (CHANNEL->length == 0 && CHANNEL->lengthEnable)
    CHANNEL->isEnabled = false;
}

void channelNoiseTickEnvelope(ChannelNoise* CHANNEL)
{
  if (CHANNEL->envelopeSweep > 0)
  {
    if (CHANNEL->envelopeCounter > 0) CHANNEL->envelopeCounter--;
    if (CHANNEL->envelopeCounter == 0)
    {
      int dir = (CHANNEL->envelopeDirection == 1) ? 1 : -1;
      CHANNEL->envelopeVolume += dir;
      if (CHANNEL->envelopeVolume < 0) CHANNEL->envelopeVolume = 0;
      if (CHANNEL->envelopeVolume > 15) CHANNEL->envelopeVolume = 15;

      CHANNEL->envelopeCounter = CHANNEL->envelopeSweep;
    }
  }
}

void channelNoiseTickSampleGenerator(ChannelNoise* CHANNEL, i32 CYCLES)
{
  CHANNEL->counter -= CYCLES;
  if (CHANNEL->counter <= 0)
  {
    CHANNEL->counter += CHANNEL->period;

    u8 bit0 = CHANNEL->lfsr & 0x01;
    u8 bit1 = (CHANNEL->lfsr >> 1) & 0x01;
    u8 xorBit = bit0 ^ bit1;

    CHANNEL->lfsr >>= 1;
    CHANNEL->lfsr |= (xorBit << 14);

    if (CHANNEL->lfsrWidth)
    {
      CHANNEL->lfsr &= ~(1 << 6);
      CHANNEL->lfsr |= (xorBit << 6);
    }

    if (CHANNEL->isEnabled)
      CHANNEL->sample = (i8)(((~CHANNEL->lfsr) & 0x1) * CHANNEL->envelopeVolume);
    else
      CHANNEL->sample = 0;
  }
}

bool channelNoiseIsEnabled(ChannelNoise* CHANNEL)
{
  return CHANNEL->isEnabled;
}

void channelNoiseDisable(ChannelNoise* CHANNEL)
{
  CHANNEL->isEnabled = false;
}
