#include "../../include/channelPulse.h"

static i32  channelPulseInternalSweep(ChannelPulse* CHANNEL);
static void channelPulseHandleTrigger(ChannelPulse* CHANNEL);

void channelPulseInit(ChannelPulse* CHANNEL)
{
  FORGE_ASSERT_MESSAGE(CHANNEL, "Cannot initialize a null channel");

  CHANNEL->waveform[0]  = 0x01;
  CHANNEL->waveform[1]  = 0x81;
  CHANNEL->waveform[2]  = 0x87;
  CHANNEL->waveform[3]  = 0x7E;
  CHANNEL->sample       = 0;

  CHANNEL->isEnabled            = false;
  CHANNEL->counter              = 0;
  CHANNEL->frequency            = 0;
  CHANNEL->wavePatternPosition  = 0;
  CHANNEL->envelopeCounter      = 0;
  CHANNEL->envelopeVolume       = 0;
  CHANNEL->sweepCounter         = 0;

  CHANNEL->NR10         = 0;
  CHANNEL->sweepTime    = 0;
  CHANNEL->sweepStep    = 0;
  CHANNEL->sweepShift   = 0;

  CHANNEL->NRX1                     = 0;
  CHANNEL->wavePatternDuty          = 0;
  CHANNEL->length                   = 0;
  CHANNEL->NRX2                     = 0;
  CHANNEL->envelopeInitialVolume    = 0;
  CHANNEL->envelopeDirection        = 0;
  CHANNEL->envelopeSweep            = 0;
  CHANNEL->NRX3periodLo             = 0;
  CHANNEL->NRX4                     = 0;

  CHANNEL->periodHi                 = 0;
  CHANNEL->trigger                  = false;
  CHANNEL->lengthEnable             = false;
}

void channelPulseSweep(ChannelPulse* CHANNEL, u8 VALUE)
{
  CHANNEL->NR10       = (VALUE & 0x7F) | 0x80; 
  CHANNEL->sweepTime  = (VALUE >> 4) & 0x7;
  CHANNEL->sweepStep  = (VALUE >> 3) & 0x1;
  CHANNEL->sweepShift = VALUE & 0x7;
}

void channelPulseSetNRx1LengthTimerDutyCycle(ChannelPulse* CHANNEL, u8 VALUE)
{
  CHANNEL->NRX1             = VALUE | 0x3F;
  CHANNEL->wavePatternDuty  = (VALUE >> 6) & 0x3;
  CHANNEL->length           = 64 - (VALUE & 0x3F);
}

void channelPulseSetNRx2EnvelopeVolume(ChannelPulse* CHANNEL, u8 VALUE)
{
  CHANNEL->NRX2                     = VALUE;
  CHANNEL->envelopeInitialVolume    = (VALUE >> 4) & 0xF;
  CHANNEL->envelopeDirection        = (VALUE >> 3) & 0x1;
  CHANNEL->envelopeSweep            = VALUE & 0x7;
}

void channelPulseSetNRx3PeriodLow(ChannelPulse* CHANNEL, u8 VALUE)
{ CHANNEL->NRX3periodLo = VALUE; }

void channelPulseSetNRx4PeriodHiControl(ChannelPulse* CHANNEL, u8 VALUE)
{
  CHANNEL->NRX4             = VALUE | 0xBF;
  CHANNEL->trigger          = (VALUE & 0x80) != 0;
  CHANNEL->lengthEnable     = (VALUE & 0x40) != 0;
  CHANNEL->periodHi         = VALUE & 0x7;

  if (CHANNEL->trigger)
  {
    CHANNEL->trigger = false;
    channelPulseHandleTrigger(CHANNEL);
  }
}

static void channelPulseHandleTrigger(ChannelPulse* CHANNEL)
{
  CHANNEL->isEnabled = true;
  if (CHANNEL->length == 0) CHANNEL->length = 64;

  CHANNEL->frequency        = ((u16)CHANNEL->periodHi << 8) | CHANNEL->NRX3periodLo;
  CHANNEL->envelopeCounter  = CHANNEL->envelopeSweep;
  CHANNEL->envelopeVolume   = CHANNEL->envelopeInitialVolume;
  CHANNEL->sweepCounter     = CHANNEL->sweepTime;

  CHANNEL->counter             = (2048 - CHANNEL->frequency) * 4;
  if (CHANNEL->trigger)
  {
    CHANNEL->trigger = false;
    if (!CHANNEL->isEnabled) CHANNEL->wavePatternPosition = 0;
  }

  if (CHANNEL->sweepShift > 0)
  {
    i32 sweep = channelPulseInternalSweep(CHANNEL);
    if (sweep > 2047) CHANNEL->isEnabled = false;
  }
}

void channelPulseTickLength(ChannelPulse* CHANNEL)
{
  if (CHANNEL->length > 0) CHANNEL->length--;
  if (CHANNEL->length == 0 && CHANNEL->lengthEnable) 
  { CHANNEL->isEnabled = false; }
}

void channelPulseTickSweep(ChannelPulse* CHANNEL)
{
  if (CHANNEL->sweepTime == 0) return;

  if (--CHANNEL->sweepCounter <= 0)
  {
    i32 sweep = channelPulseInternalSweep(CHANNEL);
    if (sweep <= 2047 && CHANNEL->sweepShift > 0)
    {
      CHANNEL->frequency    = sweep;
      CHANNEL->periodHi     = (sweep >> 8) & 0x7;
      CHANNEL->NRX3periodLo = sweep & 0xFF;
    }
    else CHANNEL->isEnabled = false;

    CHANNEL->sweepCounter = CHANNEL->sweepTime;
  }
}

static i32 channelPulseInternalSweep(ChannelPulse* CHANNEL)
{
  i32 step    = (CHANNEL->sweepStep == 1) ? -1 : 1;
  u16 freq    = CHANNEL->frequency;
  i32 shifted = freq >> CHANNEL->sweepShift;
  i32 result  = freq + step * shifted;

  return result;
}

void channelPulseTickEnvelope(ChannelPulse* CHANNEL)
{
  if (CHANNEL->envelopeSweep > 0)
  {
    if (--CHANNEL->envelopeCounter <= 0)
    {
      CHANNEL->envelopeCounter = CHANNEL->envelopeSweep;

      i32 newVol = CHANNEL->envelopeVolume + (CHANNEL->envelopeDirection ? 1 : -1);

      if (newVol >= 0 && newVol <= 15) CHANNEL->envelopeVolume = newVol;
    }
  }
}
void channelPulseTickSampleGenerator(ChannelPulse* CHANNEL, i32 CYCLES)
{
  CHANNEL->counter -= CYCLES;

  if (CHANNEL->counter <= 0)
  {
    CHANNEL->frequency              = ((u16)CHANNEL->periodHi << 8) | CHANNEL->NRX3periodLo;
    CHANNEL->counter                = (2048 - CHANNEL->frequency) * 4;
    CHANNEL->wavePatternPosition    = (CHANNEL->wavePatternPosition + 1) & 0x7;

    u8  wave    = CHANNEL->waveform[CHANNEL->wavePatternDuty];
    i32 output  = (wave >> (7 - CHANNEL->wavePatternPosition)) & 0x01;

    CHANNEL->sample = CHANNEL->isEnabled ? (i8)(output * CHANNEL->envelopeVolume) : 0;
  }
}

bool channelPulseIsEnabled(ChannelPulse* CHANNEL)
{ return CHANNEL->isEnabled; }

void channelPulseDisable(ChannelPulse* CHANNEL)
{ CHANNEL->isEnabled = false; }
