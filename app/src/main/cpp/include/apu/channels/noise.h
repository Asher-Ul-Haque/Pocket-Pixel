#pragma once
#include <common.h>

typedef struct 
{
  bool enabled;
  bool dacEnabled;
  
  // - - - Length
  u8   lengthTimer;
  bool lengthEnabled;

  // - - - Volume Envelope
  u8   initialVolume;
  u8   currentVolume;
  u8   envelopeTimer;
  u8   envelopePace;
  bool envelopeIncrease;

  // - - - Linear Feedback Shift Register (LFSR)
  u32  periodTimer;
  u16  lfsr;
  u8   clockShift;
  bool shortMode; ///< True = 7-bit mode, False = 15-bit mode
  u8   clockDivider;

  // - - - Output
  u8   outputVolume;
} NoiseChannel;

void noiseTrigger       (NoiseChannel* CHANNEL);
void noiseClockLength   (NoiseChannel* CHANNEL);
void noiseClockEnvelope (NoiseChannel* CHANNEL);
void noiseStepTimer     (NoiseChannel* CHANNEL);
