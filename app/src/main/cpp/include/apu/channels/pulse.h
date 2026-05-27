#pragma once 
#include <common.h>

typedef struct 
{
  bool enabled;
  bool dacEnabled;
  bool hasSweepHardware;

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
void pulseTrigger(PulseChannel* CHANNEL);

// - - - Frame Sequencer Clocks - - -
void pulseClockLength  (PulseChannel* CHANNEL);
void pulseClockEnvelope(PulseChannel* CHANNEL);
void pulseClockSweep   (PulseChannel* CHANNEL);

// - - - CPU M-Cycle Clock - - -
void pulseStepTimer(PulseChannel* CHANNEL);
