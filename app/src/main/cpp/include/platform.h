/**
 * @file platform.h 
 * @brief this is to provide the emulator to access to specific platform functions like file io, sound io, graphics, keyboard input etc
*/

#pragma once 
#include <common.h>
#include <ppu/ppu.h>

// - - - HARDWARE DEFINITIONS - - - 
#define AUDIO_FREQ 44100

// - - - PLATFORM CONFIGURATION STRUCTURES - - - 

/**
 * @brief Represents the frontend's aesthetic color mapping for DMG mode.
 * The platform maps the PPU's 0-3 indices to these 32-bit (RGBA8888) values.
*/
typedef struct DmgPalette 
{
  u32 color0; 
  u32 color1;
  u32 color2;
  u32 color3; 
} DmgPalette;


// - - - SUBSYSTEM INTERFACES - - - 

/// @brief File IO access provided to emulator
typedef struct CartridgeFileIO 
{
  bool (*saveRamToFile)       (const u8*  RAM_DATA,        u32 RAM_SIZE);
  bool (*loadRamFromFile)     (u8*        RAM_DATA_BUFFER, u32 BUFFER_SIZE);
  u32  (*getExpectedSaveSize) (void);
} CartridgeFileIO;

/// @brief Graphics access provided to the emulator
typedef struct RendererSystem 
{
  bool (*init)(void);

  // - - - Frontend Visual Controls
  void (*setDmgPalette)(DmgPalette  PALETTE);
  void (*enableShader) (bool        ENABLE);

  // - - - Core Interaction Hook
  void (*renderFrame) (const PpuFrame* FRAME);

  #ifdef DEBUG 
    void (*drawTileView)(const u8* VBK0, const u8* VBK1); 
  #endif
  void (*present)(void);
  void (*cleanup)(void);
} RendererSystem;

/// @brief: Keybinds 
typedef struct InputConfig
{
  // - - - key up
  u16 keyUp; 
  u16 keyDown;
  u16 keyLeft;
  u16 keyRight;
  u16 keyA;
  u16 keyB;
  u16 keySelect;
  u16 keyStart;

  u16 keyPause;
  u16 keySpeed;
  u16 keyFullscreen;

  // - - - Gamepad buttons 
  u16 padA;
  u16 padB;
  u16 padStart;
  u16 padSelect;
  u16 padUp;
  u16 padDown;
  u16 padLeft;
  u16 padRight;

  u16 padPause;
  u16 padSpeed;
  u16 padFullscreen;
} InputConfig;

/// @brief Keyboard / Gamepad access to the emulator
typedef struct InputSystem
{
  InputConfig config;

  // - - - System states 
  bool paused;
  bool doubleSpeed;
  bool fullscreen;

  void (*poll)      (bool*       IS_RUNNING);
  void (*setConfig) (InputConfig NEW_CONFIG);
} InputSystem;

/// @brief Audio access to the emulator
typedef struct AudioSystem
{
  bool (*init)        (void);
  void (*pushSamples) (const f32* SAMPLES, u32 COUNT);
  void (*cleanup)     (void);
} AudioSystem;

/// @brief: MASTER PLATFORM CONTEXT
typedef struct PlatformContext 
{
  const char*       name;
  CartridgeFileIO   fileIO;
  RendererSystem    video;
  AudioSystem       audio;
  InputSystem       input;
} PlatformContext;


// - - - Architecture Endpoints - - -

/**
 * @brief Global access to platform layer to the emualtor 
 * @return a pointer to the platform 
 * @note This will never be a null pointer
*/
PlatformContext* platformGetContext(void);

/// @brief Initialize platform systems 
void             platformInit(void);
