/**
 * @file apu/wave.h 
 * @brief The wave channel of the Game boy APU
*/
#pragma once
#include <common.h>
#include <apu/internal.h>


/// @brief Wave channel of the apu 
typedef struct 
{
  bool enabled;
  bool dacEnabled;
  
  // - - - Waveform
  u16 periodTimer;
  u16 periodValue;
  u8  positionIndex;          ///< Tracks which of the 32 samples we are on
  u8  waveRam[WAVE_RAM_SIZE]; ///< Wave channel has its own small ram

  // - - - Length
  u16  lengthTimer;   ///< Wave channel length timer goes up to 256
  bool lengthEnabled;

  // - - - Volume
  u8  volumeCode; ///< 0=Mute, 1=100%, 2=50%, 3=25%

  // - - - Output
  u8  outputVolume;
} WaveChannel;

/**
 * @brief trigger the wave channel 
 * @param CHANNEL the channel 3 of the apu 
*/
void waveTrigger    (WaveChannel* CHANNEL);

/**
 * @brief reset clock of the wave channel 
 * @param CHANNEL the channel 3 of the apu 
*/
void waveClockLength(WaveChannel* CHANNEL);

/**
 * @brief M-cycle tick for the wave channel 
 * @param CHANNEL the channel 3 of the apu
*/
void waveStepTimer  (WaveChannel* CHANNEL);

// - - - I/O hooks specifically for reading/writing the 16-byte RAM - - - 

/**
 * @brief Reads the apu wave channel ram 
 * @return The wave channel ram byte at the given address 
 * @param CHANNEL the channel 3 of the apu 
 * @param ADDR the address to read in wave channel ram 
*/
u8   waveReadRam (WaveChannel* CHANNEL, u16 ADDR);

/**
 * @brief Writes to the apu wave channel ram 
 * @param CHANNEL the channel 3 of the apu 
 * @param ADDR the address to write to in wave channel ram 
 * @param VALUE the byte which is to be written
*/
void waveWriteRam(WaveChannel* CHANNEL, u16 ADDR, u8 VALUE);
