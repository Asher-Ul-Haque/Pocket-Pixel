#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct 
{
  i8    NR30;
  i8    NR31;
  i8    NR32;
  i8    NRX4;

  u8    wavePatternRAM[16];

  bool  isEnabled;
  bool  lengthEnable;
  bool  trigger;

  i32   length;
  u8    volume;
  u16   frequency;

  i32   counter;
  u8    waveIndex;
  i8    nibble;

  u8    sample;
  u8    periodLo;
  u8    periodHi;
} ChannelWave;

FORGE_API void channelWaveInit(ChannelWave* CHANNEL);
FORGE_API void channelWaveSetVolume(f32 VOLUME);
FORGE_API void channelWaveSetNR30(ChannelWave* CHANNEL, u8 VALUE);
FORGE_API void channelWaveSetNR31(ChannelWave* CHANNEL, u8 VALUE);
FORGE_API void channelWaveSetNR32(ChannelWave* CHANNEL, u8 VALUE);
FORGE_API void channelWaveSetNRX3(ChannelWave* CHANNEL, u8 VALUE);
FORGE_API void channelWaveSetNRX4(ChannelWave* CHANNEL, u8 VALUE);
FORGE_API void channelWaveTickLength(ChannelWave* CHANNEL);
FORGE_API void channelWaveTickSampleGenerator(ChannelWave* CHANNEL, i32 CYCLES);


#ifdef __cplusplus
}
#endif
