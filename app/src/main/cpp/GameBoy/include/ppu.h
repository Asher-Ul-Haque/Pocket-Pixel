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

// - - - colors 

#define RGBA(r, g, b) ((u32)(255 << 24 | (b) << 16 | (g) << 8 | (r)))

// - - - Original Game Boy (DMG)
static const u32 PPU_COLOR_DMG[4] = 
  {
    RGBA(154, 161, 60),
    RGBA(108, 113, 42),
    RGBA(77, 81, 30),
    RGBA(31, 32, 12)
  };

// - - - Game Boy Pocket (Grayscale)
static const u32 PPU_COLOR_POCKET[4] = 
  {
    RGBA(255, 255, 255),
    RGBA(169, 169, 169),
    RGBA(84, 84, 84),
    RGBA(0, 0, 0)
  };

// - - - Game Boy Default (Classic Green)
static const u32 PPU_COLOR_DEFAULT[4] = 
  {
    RGBA(224, 248, 208),
    RGBA(136, 192, 112),
    RGBA(52, 104, 86),
    RGBA(8, 24, 32)
  };

// - - - Game Boy Color Purple Palette
static const u32 PPU_COLOR_PURPLE[4] = 
  {
    RGBA(255, 204, 255),
    RGBA(192, 96, 192),
    RGBA(96, 0, 96),
    RGBA(0, 0, 0)
  };

// - - - Sepia Palette
static const u32 PPU_COLOR_SEPIA[4] = 
  {
    RGBA(255, 240, 192),
    RGBA(208, 176, 128),
    RGBA(160, 112, 64),
    RGBA(64, 32, 0)
  };

// - - - Blue Palette
static const u32 PPU_COLOR_BLUE[4] = 
  {
    RGBA(208, 240, 255),
    RGBA(128, 192, 255),
    RGBA(32, 104, 192),
    RGBA(0, 24, 80)
  };

// - - - Teal Palette
static const u32 PPU_COLOR_TEAL[4] = 
  {
    RGBA(224, 255, 248),
    RGBA(144, 232, 208),
    RGBA(56, 136, 120),
    RGBA(0, 40, 40)
  };

// - - - Peach Palette
static const u32 PPU_COLOR_PEACH[4] = 
  {
    RGBA(255, 240, 224),
    RGBA(255, 192, 160),
    RGBA(240, 128, 96),
    RGBA(128, 32, 0)
  };

static const u32* colorSchemes[8] = 
  {
    PPU_COLOR_DEFAULT,
    PPU_COLOR_DMG,
    PPU_COLOR_POCKET,
    PPU_COLOR_PURPLE,
    PPU_COLOR_SEPIA,
    PPU_COLOR_BLUE,
    PPU_COLOR_TEAL,
    PPU_COLOR_PEACH
  };

FORGE_API void setColorScheme(u8 INDEX);

FORGE_API void doubleSpeed();


#ifdef __cplusplus
}
#endif
