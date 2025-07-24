#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "channelPulse.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APU_BUFFER_SIZE 4096

typedef struct APUcontext 
{
  // - - - Channel - - - 

  ChannelPulse channel1;


  // - - - Buffer - - - 

  i32 bufferPtr = 0;
  u8  sampleBuffer[APU_BUFFER_SIZE];
  

  // - - - Master Registers - - - 

  u8 NR50;
  u8 NR51;
  u8 NR52;


  // - - - Volume - - - 

  i32 vinLeft;
  i32 vinRight;
  i32 masterVolumeLeft;
  i32 masterVolumeRight;

  bool channel1Left;
  bool channel1Right;

  bool isEnabled;

  i32 sampleCounter;
  i32 frameSequencerCounter;
  i32 frameSequencerStep;
} APUcontext;

FORGE_API void          apuInit();
FORGE_API APUcontext*   apuGetContext();
FORGE_API void          apuUpdate(i32 CYCLES);
FORGE_API void          apuWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8            apuRead(u16 ADDRESS);


#ifdef __cplusplus
}
#endif
