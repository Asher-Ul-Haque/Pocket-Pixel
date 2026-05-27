/**
 * @file apu/noise.h 
 * @brief The noise channel of the Game boy APU
*/
#pragma once 
#include <common.h>

/// @brief the definition of the pulse channels for the apu 
typedef struct 
{
  bool enabled;
  bool dacEnabled;
  bool hasSweepHardware; ///< true for channel 1, false for channel 2

  // - - - Waveform
  u8  dutyPattern;
  u8  dutyPosition;
  u16 periodTimer;
  u16 periodValue;

  // - - - Length
  u8   lengthTimer;
  bool lengthEnabled;

  // - - - Volume Envelope
  u8   initialVolume;
  u8   currentVolume;
  u8   envelopeTimer;
  u8   envelopePace;
  bool envelopeIncrease;

  // - - - Hardware Sweep (CH1 Only)
  u8   sweepPace;
  bool sweepDecrease;
  u8   sweepShift;
  u8   sweepTimer;
  u16  sweepShadow;
  bool sweepEnabled;

  // - - - Output
  u8   outputVolume;
} PulseChannel;

// - - - Component Lifecycle - - -

/**
 * @brief triggers the pulse channel 
 * @param CHANNEL one of the pulse channels 
*/
void pulseTrigger(PulseChannel* CHANNEL);

// - - - Frame Sequencer Clocks - - -

/**
 * @brief set clock length of the pulse channel 
 * @param CHANNEL one of the pulse channels 
*/
void pulseClockLength  (PulseChannel* CHANNEL);

/**
 * @brief envelope the pulse clock 
 * @param CHANNEL one of the pulse channels 
*/
void pulseClockEnvelope(PulseChannel* CHANNEL);

/**
 * @brief sweep audio smoothly 
 * @param CHANNEL the channel 1 of the apu 
 * @warning CHANNEL must be channel 1 and not channel 2
*/
void pulseClockSweep   (PulseChannel* CHANNEL);

// - - - CPU M-Cycle Clock - - -

/**
 * @brief M-Cycle tick for pulse channel
 * @param CHANNEL one of the pulse channels
*/
void pulseStepTimer(PulseChannel* CHANNEL);
