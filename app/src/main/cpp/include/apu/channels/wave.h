#pragma once
#include <common.h>
#include <apu/internal.h>

typedef struct 
{
  bool enabled;
  bool dacEnabled;
  
  // - - - Waveform
  u16 periodTimer;
  u16 periodValue;
  u8  positionIndex;          ///< Tracks which of the 32 samples we are on
  u8  waveRam[WAVE_RAM_SIZE];

  // - - - Length
  u16  lengthTimer;   ///< Wave channel length timer goes up to 256
  bool lengthEnabled;

  // - - - Volume
  u8  volumeCode; ///< 0=Mute, 1=100%, 2=50%, 3=25%

  // - - - Output
  u8  outputVolume;
} WaveChannel;

void waveTrigger    (WaveChannel* CHANNEL);
void waveClockLength(WaveChannel* CHANNEL);
void waveStepTimer  (WaveChannel* CHANNEL);

// - - - I/O hooks specifically for reading/writing the 16-byte RAM
u8   waveReadRam (WaveChannel* CHANNEL, u16 ADDR);
void waveWriteRam(WaveChannel* CHANNEL, u16 ADDR, u8 VALUE);
