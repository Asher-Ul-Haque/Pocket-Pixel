#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  u8  waveform[4];
  i8  sample;

  bool isEnabled;
  i64  counter;
  u16  frequency;
  i32  wavePatternPosition;
  i32  envelopeCounter;
  i32  envelopeVolume;
  i32  sweepCounter;

  u8   NR10;
  i32  sweepTime;
  i32  sweepStep;
  i32  sweepShift;

  u8   NRX1;
  i32  wavePatternDuty;
  i32  length;

  u8   NRX2;
  i32  envelopeInitialVolume;
  i32  envelopeDirection;
  i32  envelopeSweep;

  u8   NRX3periodLo;
  u8   NRX4;
  u8   periodHi;
  bool trigger;
  bool lengthEnable;
} ChannelPulse;

FORGE_API void channelPulseInit(ChannelPulse* CHANNEL);
FORGE_API void channelPulseSweep(ChannelPulse* CHANNEL, u8 VALUE);
FORGE_API void channelPulseSetNRx1LengthTimerDutyCycle(ChannelPulse* CHANNEL, u8 VALUE);
FORGE_API void channelPulseSetNRx2EnvelopeVolume(ChannelPulse* CHANNEL, u8 VALUE);
FORGE_API void channelPulseSetNRx3PeriodLow(ChannelPulse* CHANNEL, u8 VALUE);
FORGE_API void channelPulseSetNRx4PeriodHiControl(ChannelPulse* CHANNEL, u8 VALUE);

FORGE_API void channelPulseTickLength(ChannelPulse* CHANNEL);
FORGE_API void channelPulseTickSweep(ChannelPulse* CHANNEL);
FORGE_API void channelPulseTickEnvelope(ChannelPulse* CHANNEL);
FORGE_API void channelPulseTickSampleGenerator(ChannelPulse* CHANNEL, i32 CYCLES);

FORGE_API bool channelPulseIsEnabled(ChannelPulse* CHANNEL);
FORGE_API void channelPulseDisable(ChannelPulse* CHANNEL);


#ifdef __cplusplus
}
#endif
