#include "../include/lcd.h"
#include "../include/ppu.h"
#include "../include/directMemAccess.h"


static LCDcontext ctx;
static u64 colorsDefault[4] = 
  {
    0xFF0F380F, // - - - dark green
    0xFF206230, // - - - medium green
    0xFF8BAC0F, // - - - light green 
    0xFF9BBC0F  // - - - pale green
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

  for (int i = 0; i < 4; ++i)
  {
    ctx.bgColors[i]   = colorsDefault[i];
    ctx.sp1Colors[i]  = colorsDefault[i];
    ctx.sp2Colors[i]  = colorsDefault[i];
  }
}

LCDcontext* lcdGetContext()
{ return &ctx; }

void updatePallete(u8 PALLETE_DATA, u8 PALLETE)
{
  u32* pColros = ctx.bgColors;

  switch (PALLETE)
  {
    case 1 : 
      pColros = ctx.sp1Colors;
      break;

    case 2 :
      pColros = ctx.sp2Colors;
      break;
  }

  pColros[0] = colorsDefault[PALLETE_DATA & 0b11];
  pColros[1] = colorsDefault[(PALLETE_DATA >> 2) & 0b11];
  pColros[2] = colorsDefault[(PALLETE_DATA >> 4) & 0b11];
  pColros[3] = colorsDefault[(PALLETE_DATA >> 6) & 0b11];
}

u8 lcdRead(u16 ADDRESS)
{
  u8  offset = (ADDRESS - 0xFF40);
  u8* p      = (u8 *) &ctx;
  return p[offset];
}

void lcdWrite(u16 ADDRESS, u8 VALUE)
{
  u8 offset = (ADDRESS - 0xFF40);
  u8* p     = (u8 *) &ctx;
  p[offset] = VALUE;

  if      (offset == 6)       dmaStart(VALUE);
  else if (ADDRESS == 0xFF47) updatePallete(VALUE, 0);
  else if (ADDRESS == 0xFF48) updatePallete(VALUE & 0b11111100, 1);
  else if (ADDRESS == 0xFF49) updatePallete(VALUE & 0b11111100, 1);
}
