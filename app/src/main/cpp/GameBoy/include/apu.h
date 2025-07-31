#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "channelNoise.h"
#include "channelPulse.h"
#include "channelWave.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APU_BUFFER_SIZE 4096

typedef struct APUcontext 
{
  // - - - Channel Structures - - - 
  ChannelPulse channel1;
  ChannelPulse channel2;
  ChannelWave  channel3;
  ChannelNoise channel4;

  // - - - Audio Ring Buffer - - - 
  i8  sampleBuffer[APU_BUFFER_SIZE];
  i16 bufferPtr;

  // - - - Master Registers (used for APU control and status) - - - 
  u8 NR50; 
  u8 NR51; 
  u8 NR52; 

  // - - - Volume and Panning Flags (derived from NR50/NR51 for easier access) - - - 
  i32 vinLeft;
  i32 vinRight;
  i32 masterVolumeLeft;
  i32 masterVolumeRight;

  f32 volumes[5];

  bool channel1Left;
  bool channel1Right;
  bool channel2Left;
  bool channel2Right;
  bool channel3Left;
  bool channel3Right;
  bool channel4Left;
  bool channel4Right;

  bool isEnabled; 

  // - - - Internal APU Timing Counters - - - 
  i32 sampleCounter;        
  i32 frameSequencerCounter; 
  i32 frameSequencerStep;    
} APUcontext;

// - - - APU Public API Functions - - - 
FORGE_API APUcontext*   apuGetContext();
FORGE_API void          apuInit(f32* VOLUMES);
FORGE_API void          apuSetVolume(f32* VOLUMES);
FORGE_API void          apuUpdate(i32 CYCLES); 
FORGE_API void          apuWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8            apuRead(u16 ADDRESS); 

#ifdef __cplusplus
}
#endif
