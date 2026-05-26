#pragma once 
#include <apu/internal.h>

typedef struct 
{
  bool enabled;
} PulseChannel;

typedef struct 
{
  bool enabled;
} WaveChannel;

typedef struct 
{
  bool enabled;
} NoiseChannel;

/// @brief: Master APU Context 
typedef struct 
{
  PulseChannel channel1;
  PulseChannel channel2;
  WaveChannel  channel3;
  NoiseChannel channel4;

  f32 sampleBuffer[AUDIO_BUFFER_SIZE];
  u32 bufferIndex;

  f32 sampleAccumulator;
  f32 testTonePhase;
} ApuContext;

ApuContext* apuGetContext(void);
void        apuInit(void);
void        apuTick(void);
