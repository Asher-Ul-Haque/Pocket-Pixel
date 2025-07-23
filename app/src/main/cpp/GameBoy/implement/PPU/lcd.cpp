#include "../../include/lcd.h"
#include "../../include/directMemAccess.h"

static LCDcontext ctx;

#define RGBA(r, g, b) ((u32)(255 << 24 | b << 16 | g << 8 | r))

static u32 colorsDefault[4] =
  {
    RGBA(224,248,208),  // - - - Lightest green
    RGBA(136,192,112),  // - - - Light green
    RGBA(52,104,86),    // - - - Dark green
    RGBA(8,24,32),      // - - - Darkest green
  };

void lcdInit() 
{
  ctx.control       = 0x91;
  ctx.scrollX       = 0;
  ctx.scrollY       = 0;
  ctx.ly            = 0;
  ctx.lyCompare     = 0;
  ctx.bgPalette     = 0xFC;
  ctx.objPalette[0] = 0xFF;
  ctx.objPalette[1] = 0xFF;
  ctx.windowY       = 0;
  ctx.windowX       = 0;

  for (i32 i = 0; i < 4; i++) 
  {
    ctx.bgColors[i]  = colorsDefault[i];
    ctx.sp1Colors[i] = colorsDefault[i];
    ctx.sp2Colors[i] = colorsDefault[i];
  }
}

LCDcontext* lcdGetContext() 
{ return &ctx; }

u8 lcdRead(u16 ADDRESS) 
{
  u8  offset = (ADDRESS - 0xFF40);
  u8* p      = (u8 *)&ctx;

  return p[offset];
}

void updatePalette(u8 PALETTE_DATA, u8 PAL) 
{
  u32* pColors = ctx.bgColors;

  switch(PAL) 
  {
    case 1 : { pColors = ctx.sp1Colors; break; }
    case 2 : { pColors = ctx.sp2Colors; break; }
  }

  pColors[0] = colorsDefault[PALETTE_DATA & 0b11];
  pColors[1] = colorsDefault[(PALETTE_DATA >> 2) & 0b11];
  pColors[2] = colorsDefault[(PALETTE_DATA >> 4) & 0b11];
  pColors[3] = colorsDefault[(PALETTE_DATA >> 6) & 0b11];
}

void lcdWrite(u16 ADDRESS, u8 VALUE) 
{
  u8  offset = (ADDRESS - 0xFF40);
  u8* p      = (u8 *)&ctx;
  p[offset]  = VALUE;

  if (offset == 6) dmaStart(VALUE);

  if      (ADDRESS == 0xFF47) updatePalette(VALUE, 0);
  else if (ADDRESS == 0xFF48) updatePalette(VALUE & 0b11111100, 1);
  else if (ADDRESS == 0xFF49) updatePalette(VALUE & 0b11111100, 2);
}
