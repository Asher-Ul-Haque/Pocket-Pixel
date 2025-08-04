#pragma once
#include "../../defines.h" // For u8, u16, u32, i32, FORGE_API, BIT, BIT_SET, BIT_CLEAR
#include "../../ForgeLibrary/include/logger.h" // For FORGE_LOG_ERROR, etc.
#include "../../ForgeLibrary/include/asserts.h" // For FORGE_ASSERT_MESSAGE

#ifdef __cplusplus
extern "C" {
#endif

// - - - Constatns - - - 

static const i32 SCREEN_WIDTH         = 160;
static const i32 SCREEN_HEIGHT        = 144;
static const i32 SCREEN_VBLANK_HEIGHT = 153; // - - - LY goes up to 153 (0-153 = 154 lines total)

static const i32 OAM_CYCLES          = 80;
static const i32 VRAM_CYCLES         = 172;
static const i32 HBLANK_CYCLES       = 204;
static const i32 SCANLINE_CYCLES     = 456; // - - - OAM + VRAM + HBLANK = 80 + 172 + 204 = 456


// - - - Enums and Structs - - -

// - - - PPU Modes
typedef enum 
{
  MODE_HBLANK = 0,
  MODE_VBLANK = 1,
  MODE_OAM    = 2,
  MODE_VRAM   = 3
} PPUMode;

// - - - Global PPU Context Structure This struct holds all the PPU's internal state, registers, and buffers.
typedef struct 
{
  // - - - PPU Registers (from FF40-FF4B)
  u8 lcdc; // - - - FF40 - LCDC - LCD Control
  u8 stat; // - - - FF41 - STAT - LCDC Status
  u8 scy;  // - - - FF42 - SCY - Scroll Y
  u8 scx;  // - - - FF43 - SCX - Scroll X
  u8 ly;   // - - - FF44 - LY - LCDC Y-Coordinate (Read-only for CPU, writing resets to 0)
  u8 lyc;  // - - - FF45 - LYC - LY Compare
  u8 bgp;  // - - - FF47 - BGP - BG Palette Data
  u8 obp0; // - - - FF48 - OBP0 - Object Palette 0 Data
  u8 obp1; // - - - FF49 - OBP1 - Object Palette 1 Data
  u8 wy;   // - - - FF4A - WY - Window Y Position
  u8 wx;   // - - - FF4B - WX - Window X Position minus 7 (adjusted in rendering)

  // - - - Internal PPU State Counters
  i32 scanlineCounter;    // - - - Accumulates cycles for current mode
  u8  windowInternalLine; // - - - Internal Y-coordinate for the window layer (0-143)

  // - - - Framebuffer
  u32 frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; 

  // - - - Cached Palettes (ARGB format)
  u32 backgroundPalette[4];
  u32 objectPalette0[4];
  u32 objectPalette1[4];

  // - - - LCDC derived state
  bool isEnabled; 

  // - - - Sprite Ordering Buffer 
  i32 orderBuffer[40 + 1]; 

  // - - - memory
  u8 VRAM[0x2000]; 
  u8 OAM[0xA0];    
} PPUContext;



// - - - FUNCTIONS - - -

FORGE_API PPUContext* ppuGetContext();
FORGE_API int         ppuGetCpuSpeedMultiplier();
FORGE_API void        ppuInit();
FORGE_API void        ppuReset();
FORGE_API u8          ppuRead(u16 ADDRESS);
FORGE_API void        ppuWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8          ppuVRAMread(u16 ADDRESS);
FORGE_API void        ppuVRAMwrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8          ppuOAMread(u16 ADDRESS);
FORGE_API void        ppuOAMwrite(u16 ADDRESS, u8 VALUE);
FORGE_API void        ppuTick();

FORGE_API void ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE_BYTE);
FORGE_API void ppuHandleCoincidenceFlag();
FORGE_API void ppuUpdateStatMode(PPUMode MODE);
FORGE_API void ppuDrawScanLine();
FORGE_API void ppuRenderBG();
FORGE_API void ppuRenderSpritesBuffer();


FORGE_API void        setColorScheme(u8 INDEX);
FORGE_API const u32*  getColorScheme();
FORGE_API void        ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE);


#ifdef __cplusplus
}
#endif
