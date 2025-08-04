#include "../../include/ppu.h"

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

static u8 colorSchemeIndex = 0;

const u32*  getColorScheme() 
{ 
  return colorSchemes[colorSchemeIndex]; 
}

void setColorScheme(u8 INDEX)  
{ 
  PPUContext* ctx  = ppuGetContext();
  colorSchemeIndex = INDEX % 8; 
  ppuCachePalette(ctx->backgroundPalette,   getColorScheme(), ctx->bgp);
  ppuCachePalette(ctx->objectPalette0,      getColorScheme(), ctx->obp0);
  ppuCachePalette(ctx->objectPalette1,      getColorScheme(), ctx->obp1);
}

// - - - Palette Caching
void ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE) 
{
  CACHED_PALLETE[0] = COLORS_SOURCE[PALLETE & 0x3];
  CACHED_PALLETE[1] = COLORS_SOURCE[(PALLETE >> 2) & 0x3];
  CACHED_PALLETE[2] = COLORS_SOURCE[(PALLETE >> 4) & 0x3];
  CACHED_PALLETE[3] = COLORS_SOURCE[(PALLETE >> 6) & 0x3];
}
