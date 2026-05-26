#pragma once 
#include <common.h>

typedef struct 
{
  bool enabled;
  bool dacEnabled;

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

  // - - - Output
  u8   outputVolume;
} PulseChannel;

// - - - Component Lifecycle - - -
void pulseTrigger(PulseChannel* CHANNEL);

// - - - Frame Sequencer Clocks - - -
void pulseClockLength  (PulseChannel* CHANNEL);
void pulseClockEnvelope(PulseChannel* CHANNEL);

// - - - CPU M-Cycle Clock - - -
void pulseStepTimer(PulseChannel* CHANNEL);
