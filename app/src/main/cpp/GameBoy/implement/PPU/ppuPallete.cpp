#include "../../include/ppu.h"

// - - - colors 

#define RGBA(r, g, b) ((u32)(255 << 24 | (b) << 16 | (g) << 8 | (r)))

// - - - Game Boy Default (Classic Green)
static const u32 PPU_COLOR_DEFAULT[4] =
  {
    RGBA(224, 248, 208),
    RGBA(136, 192, 112),
    RGBA(52, 104, 86),
    RGBA(8, 24, 32)
  };

// - - - Original Game Boy (DMG)
static const u32 PPU_COLOR_AUTHENTIC[4] =
  {
    RGBA(154, 161, 60),
    RGBA(108, 113, 42),
    RGBA(77, 81, 30),
    RGBA(31, 32, 12)
  };

// - - - Ice cream : Kerrie Lake
static const u32 PPU_COLOR_ICE_CREAM[4] =
    {
            RGBA(0xFF, 0xF6, 0xD3),
            RGBA(0xF9, 0xA8, 0x75),
            RGBA(0xEB, 0x6B, 0x6F),
            RGBA(0x7C, 0x3F, 0x58)
    };

// - - - Hollow : Poltergasm
static const u32 PPU_COLOR_HOLLOW[4] =
        {
                RGBA(0xFA, 0xFB, 0xF6),
                RGBA(0xC6, 0xB7, 0xBE),
                RGBA(0x56, 0x5A, 0x75),
                RGBA(0x0F, 0x0F, 0x1B)
        };

// - - - Space Haze : WildLeoKnight
static const u32 PPU_COLOR_SPACE_HAZE[4] =
        {
                RGBA(0xF8, 0xE3, 0xC4),
                RGBA(0xCC, 0x34, 0x95),
                RGBA(0x6B, 0x1F, 0xB1),
                RGBA(0x0B, 0x06, 0x30)
        };

// - - - Honey : Anubi
static const u32 PPU_COLOR_HONEY[4] =
        {
                RGBA(0xE9, 0xF5, 0xDA),
                RGBA(0xF0, 0xB6, 0x95),
                RGBA(0x87, 0x72, 0x86),
                RGBA(0x3E, 0x3A, 0x42)
        };

// - - - Coffee : Mahyellaw
static const u32 PPU_COLOR_COFFEE[4] =
        {
                RGBA(0xCC, 0x9E, 0x7A),
                RGBA(0x99, 0x74, 0x5C),
                RGBA(0x73, 0x4D, 0x45),
                RGBA(0x4D, 0x30, 0x2E)
        };

// - - - RED IS DEAD : Devine Devine
static const u32 PPU_COLOR_COMRADE[4] =
        {
                RGBA(0xFF, 0xFC, 0xFE),
                RGBA(0xFF, 0x00, 0x15),
                RGBA(0x86, 0x00, 0x20),
                RGBA(0x11, 0x07, 0x0A)
        };

// - - - SNOWFLAKE 4 : Yousurname
static const u32 PPU_COLOR_SNOWFLAKE[4] =
        {
                RGBA(0xE7, 0xED, 0xEB),
                RGBA(0x8E, 0xCE, 0xCE),
                RGBA(0x62, 0xA1, 0xC7),
                RGBA(0x3F, 0x6E, 0xCC)
        };

// - - - MŒBIUS : Rabbitkng
static const u32 PPU_COLOR_RABBIT[4] =
        {
                RGBA(0xF1, 0xE0, 0xCD),
                RGBA(0xFF, 0xA4, 0x9A),
                RGBA(0xDA, 0x34, 0x67),
                RGBA(0x35, 0x33, 0x3F)
        };


static const u32* colorSchemes[10] =
  {
    PPU_COLOR_DEFAULT,
    PPU_COLOR_AUTHENTIC,
    PPU_COLOR_ICE_CREAM,
    PPU_COLOR_HOLLOW,
    PPU_COLOR_SPACE_HAZE,
    PPU_COLOR_HONEY,
    PPU_COLOR_COFFEE,
    PPU_COLOR_COMRADE,
    PPU_COLOR_SNOWFLAKE,
    PPU_COLOR_RABBIT
  };

static u8 colorSchemeIndex = 0;

const u32*  getColorScheme() 
{ 
  return colorSchemes[colorSchemeIndex]; 
}

void setColorScheme(u8 INDEX)  
{ 
  PPUContext* ctx  = ppuGetContext();
  colorSchemeIndex = INDEX % 10;
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
