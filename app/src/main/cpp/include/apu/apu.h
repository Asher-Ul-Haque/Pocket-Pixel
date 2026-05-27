#pragma once

/**
  * @file apu.h 
  * @brief Master APU header file, this is for saving state of the apu or master mixer 
*/
#include <apu/internal.h>
#include <apu/channels/pulse.h>
#include <apu/channels/wave.h>
#include <apu/channels/noise.h>

/// @brief Master APU Context 
typedef struct 
{
  PulseChannel channel1; ///< Pulse channel 1 with sweeping
  PulseChannel channel2; ///< Pulse channel 2
  WaveChannel  channel3; ///< Wave channel for smooth effects 
  NoiseChannel channel4; ///< Noise channel for percussion

  // - - - Customization 
  f32 speedMultiplier;  ///< How fast to run the apu

  // - - - Output 
  f32 sampleBuffer[AUDIO_BUFFER_SIZE]; ///< The buffer of audio samples
  u32 bufferIndex;                     ///< circular buffer head 
  f32 sampleAccumulator; 

  // - - - Frame sequencing 
  u32 frameSequencerTimer;
  u8  frameSequencerStep;

  // - - - Mixer 
  u8    masterVolumeLeft;     ///< How much in left speaker
  u8    masterVolumeRight;    ///< How much in right speaker
  u8    panningMap;
  bool  audioEnabled;         ///< Whether the apu is on or not
} ApuContext;

/**
 * @brief Global access to the apu context 
 * @return A pointer to the global apu context 
 * @note This will never return a null pointer
*/
ApuContext* apuGetContext(void);

/// @brief Initialize the apu 
void        apuInit(void);

/// @brief M cycle tick in apu
void        apuTick(void);

/**
 * @brief set the speed of apu 
 * @param MULTIPLIER a float representing how fast to run the apu 
 * @note  The minimum value it accepts is 0.01f
*/
void        apuSetSpeed(f32 MULTIPLIER);

/**
 * @brief Bus access to the apu (read)
 * @return a byte representing the value at the addr 
 * @param ADDR where to read from
*/
u8    apuRead(u16 ADDR);

/**
 * @brief Bus access to the apu (write)
 * @param ADDR where to write to 
 * @param VALUE what to write 
 * @note This sets the apu behavior
*/
void  apuWrite(u16 ADDR, u8 VALUE);
