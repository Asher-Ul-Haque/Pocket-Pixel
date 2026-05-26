#pragma once 
#include <apu/internal.h>
#include <apu/channels/pulse.h>
#include <apu/channels/wave.h>
#include <apu/channels/noise.h>

/// @brief: Master APU Context 
typedef struct 
{
  PulseChannel channel1;
  PulseChannel channel2;
  WaveChannel  channel3;
  NoiseChannel channel4;

  // - - - Customization 
  f32 speedMultiplier;

  // - - - Output 
  f32 sampleBuffer[AUDIO_BUFFER_SIZE];
  u32 bufferIndex;
  f32 sampleAccumulator;

  // - - - Frame sequencing 
  u32 frameSequencerTimer;
  u8  frameSequencerStep;

  // - - - Mixer 
  u8    masterVolumeLeft;
  u8    masterVolumeRight;
  u8    panningMap;
  bool  audioEnabled;
} ApuContext;

ApuContext* apuGetContext(void);
void        apuInit(void);
void        apuTick(void);
void        apuSetSpeed(f32 MULTIPLIER);

u8    apuRead(u16 ADDR);
void  apuWrite(u16 ADDR, u8 VALUE);
