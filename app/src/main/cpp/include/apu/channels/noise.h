/**
 * @file apu/noise.h 
 * @brief The noise channel of the Game boy APU
*/

#pragma once
#include <common.h>

/// @brief The definition of the noise channel
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

/**
 * @brief Triggers the noise channel 
 * @param CHANNEL the noise channel of the apu 
*/
void noiseTrigger       (NoiseChannel* CHANNEL);

/**
 * @brief Updates the clock for apu 
 * @param CHANNEL the noise channel of the apu 
*/
void noiseClockLength   (NoiseChannel* CHANNEL);

/**
 * @brief Envelopes the apu clock 
 * @param CHANNEL the noise channel of the apu 
*/
void noiseClockEnvelope (NoiseChannel* CHANNEL);

/**
 * @brief Step the apu noise timer 
 * @param CHANNEL the noise channel of the apu
*/
void noiseStepTimer     (NoiseChannel* CHANNEL);
