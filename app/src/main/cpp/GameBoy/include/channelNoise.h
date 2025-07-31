#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  u8 NR41;
  u8 NR42;
  u8 NR43;
  u8 NR44;

  i32 length;
  i32 envelopeInitialVolume;
  i32 envelopeDirection;
  i32 envelopeSweep;
  i32 envelopeCounter;
  i32 envelopeVolume;

  i32 clockShift;
  i32 lfsrWidth;
  i32 clockDivider;
  i32 period;
  i32 counter;

  u16 lfsr;

  bool isEnabled;
  bool trigger;
  bool lengthEnable;

  i8  sample;
} ChannelNoise;

FORGE_API void channelNoiseInit(ChannelNoise* CHANNEL);
FORGE_API void channelNoiseSetNR41(ChannelNoise* CHANNEL, u8 VALUE);
FORGE_API void channelNoiseSetNR42(ChannelNoise* CHANNEL, u8 VALUE);
FORGE_API void channelNoiseSetNR43(ChannelNoise* CHANNEL, u8 VALUE);
FORGE_API void channelNoiseSetNR44(ChannelNoise* CHANNEL, u8 VALUE);

FORGE_API void channelNoiseTickLength(ChannelNoise* CHANNEL);
FORGE_API void channelNoiseTickEnvelope(ChannelNoise* CHANNEL);
FORGE_API void channelNoiseTickSampleGenerator(ChannelNoise* CHANNEL, i32 CYCLES);
FORGE_API bool channelNoiseIsEnabled(ChannelNoise* CHANNEL);
FORGE_API void channelNoiseDisable(ChannelNoise* CHANNEL);

#ifdef __cplusplus
}
#endif
