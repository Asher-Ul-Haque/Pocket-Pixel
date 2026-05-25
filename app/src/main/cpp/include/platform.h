#pragma once 
#include <common.h>
#include <ppu/ppu.h>

// - - - HARDWARE DISPLAY DEFINITIONS - - - 

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

typedef struct CartridgeFileIO 
{
  bool (*saveRamToFile)       (const u8*  ramData,        u32 ramSize);
  bool (*loadRamFromFile)     (u8*        ramDataBuffer,  u32 bufferSize);
  u32  (*getExpectedSaveSize) (void);
} CartridgeFileIO;

typedef struct RendererSystem 
{
  bool (*init)(void);

  // - - - Frontend Visual Controls
  void (*setDmgPalette)(DmgPalette  PALETTE);
  void (*enableShader) (bool        ENABLE);

  // - - - Core Interaction Hook
  void (*renderFrame)(const PpuFrame* FRAME);
  void (*drawTileView)(const u8* VBK0, const u8* VBK1);
  void (*present)(void);
  void (*cleanup)(void);
} RendererSystem;

typedef struct InputSystem
{
  void (*poll)(bool* IS_RUNNING);
} InputSystem;


// - - - MASTER PLATFORM CONTEXT - - -

typedef struct PlatformContext 
{
  const char*       name;
  CartridgeFileIO   fileIO;
  RendererSystem    video;
  InputSystem       input;
} PlatformContext;


// - - - Architecture Endpoints - - -

PlatformContext* platformGetContext(void);
void             platformInit(void);
