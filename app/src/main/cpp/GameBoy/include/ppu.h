#pragma once
#include "../../defines.h" // u8, u16, u32, i32, BIT, etc.
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

// - - - Constants - - -
static const i32 SCREEN_WIDTH         = 160;
static const i32 SCREEN_HEIGHT        = 144;
static const i32 SCREEN_VBLANK_HEIGHT = 153;

static const i32 OAM_CYCLES      = 80;
static const i32 VRAM_CYCLES     = 172;
static const i32 HBLANK_CYCLES   = 204;
static const i32 SCANLINE_CYCLES = 456;

// - - - PPU Modes - - -
typedef enum
{
  MODE_HBLANK = 0,
  MODE_VBLANK = 1,
  MODE_OAM    = 2,
  MODE_VRAM   = 3
} PPUMode;

// - - - Global framebuffer pointer used by renderer
// - - - PPU Context - - -
typedef struct
{
  // - - - Registers
  u8 lcdc;
  u8 stat;
  u8 scy;
  u8 scx;
  u8 ly;
  u8 lyc;
  u8 bgp;
  u8 obp0;
  u8 obp1;
  u8 wy;
  u8 wx;

  // - - - Latched scroll values for tear-free rendering
  u8 scrollX_latched;
  u8 scrollY_latched;

  // - - - Internal state
  i32 scanlineCounter;
  u8  windowInternalLine;

  // - - - Framebuffers (double buffering)
  u32 frameBufferFront[SCREEN_WIDTH * SCREEN_HEIGHT];
  u32 frameBufferBack[SCREEN_WIDTH * SCREEN_HEIGHT];
  u32* frameBuffer;

  // - - - Cached palettes
  u32 backgroundPalette[4];
  u32 objectPalette0[4];
  u32 objectPalette1[4];

  bool isEnabled;

  // - - - Sprite ordering
  i32 orderBuffer[40 + 1];

  // - - - VRAM / OAM
  u8 VRAM[0x2000];
  u8 OAM[0xA0];

} PPUContext;

// - - - Public API - - -
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

FORGE_API void        ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE_BYTE);
FORGE_API void        ppuHandleCoincidenceFlag();
FORGE_API void        ppuUpdateStatMode(PPUMode MODE);
FORGE_API void        ppuDrawScanLine();
FORGE_API void        ppuRenderBG();
FORGE_API void        ppuRenderSpritesBuffer();

FORGE_API void        setColorScheme(u8 INDEX);
FORGE_API const u32*  getColorScheme();
FORGE_API void        ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE);

#ifdef __cplusplus
}
#endif
