#include "bus.h"
#include "platform.h"
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <string.h>

static PpuContext ctx;
static u8         frameCounter = 0;

PpuContext* ppuGetContext(void) { return &ctx; }

void ppuInit(void)
{
  memset(&ctx, 0, sizeof(PpuContext));
  ctx.mode = PPU_MODE_OAM_SCAN;

  // - - - default hardware register values on bootup 
  ctx.registers.lcdc = 0x91;
  ctx.registers.stat = 0x85;
  ctx.registers.bgp  = 0xFC;
  ctx.registers.obp0 = 0xFF;
  ctx.registers.obp1 = 0xFF;
}

u8 ppuRead(u16 ADDRESS)
{
  // - - - 1. VRAM Reading 
  if (ADDRESS >= 0x8000 && ADDRESS <= 0x9FFF)
  {
    return ctx.vram[0][ADDRESS - 0x8000];
  }

  // - - - 2. OAM Reading
  if (ADDRESS >= 0xFE00 && ADDRESS <= 0xFE9F)
  { return ctx.oam[ADDRESS - 0xFE00]; }

  switch (ADDRESS)
  {
    case LCDC  : return ctx.registers.lcdc;
    case STAT  : return ctx.registers.stat;
    case SCY   : return ctx.registers.scy;
    case SCX   : return ctx.registers.scx;
    case LY    : return ctx.registers.ly;
    case LYC   : return ctx.registers.lyc;
    case DMA   : return ctx.registers.dma;
    case BGP   : return ctx.registers.bgp;
    case OBP_0 : return ctx.registers.obp0;
    case OBP_1 : return ctx.registers.obp1;
    case WY    : return ctx.registers.wy;
    case WX    : return ctx.registers.wx;

    default: break;
  }

  return OPEN_BUS_VALUE;
}

void ppuWrite(u16 ADDRESS, u8 VALUE)
{
  // - - - 1. vram writing 
  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END)
  {
    ctx.vram[0][ADDRESS - BUS_ADDR_VRAM_START] = VALUE;
    return;
  }

  // - - - 2. OAM writing 
  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END)
  {
    ctx.oam[ADDRESS - BUS_ADDR_OAM_START] = VALUE;
    return;
  }

  // - - - 3. Register Encoding Range 
  switch (ADDRESS)
  {
    case LCDC   : ctx.registers.lcdc = VALUE;                                         break;
    case STAT   : ctx.registers.stat = (ctx.registers.stat & 0x07) | (VALUE & 0xF8);  break;
    case SCY    : ctx.registers.scy  = VALUE;                                         break;
    case SCX    : ctx.registers.scx  = VALUE;                                         break;
    case LY     :                                                                     break; // - - - read only 
    case LYC    : ctx.registers.lyc  = VALUE;                                         break;
    case DMA    : ctx.registers.dma  = VALUE;                                         break;
    case BGP    : ctx.registers.bgp  = VALUE;                                         break;
    case OBP_0  : ctx.registers.obp0 = VALUE;                                         break;
    case OBP_1  : ctx.registers.obp1 = VALUE;                                         break;
    case WX     : ctx.registers.wx   = VALUE;                                         break;
    case WY     : ctx.registers.wy   = VALUE;                                         break;

    default: break;
  }
}

void ppuTick(u32 DOTS)
{
  ctx.dotCount += DOTS;

  switch (ctx.mode)
  {
    case PPU_MODE_OAM_SCAN:
      if (ctx.dotCount >= 80)
      {
        ctx.dotCount -= 80;
        ctx.mode      = PPU_MODE_DRAWING;
      }
      break;

    case PPU_MODE_DRAWING:
      if (ctx.dotCount >= 172)
      {
        ctx.dotCount -= 172;
        ctx.mode      = PPU_MODE_HBLANK;
      }
      break;

    case PPU_MODE_HBLANK:
      if (ctx.dotCount >= 204)
      {
        ctx.dotCount -= 204;
        ctx.registers.ly++;

        if (ctx.registers.ly == 144)
        {
          ctx.mode = PPU_MODE_VBLANK;

          // - - - Verification
          frameCounter++;
          ctx.currentFrame.palettes.dmg.bgp   = ctx.registers.bgp;
          ctx.currentFrame.palettes.dmg.obp0  = ctx.registers.obp0;
          ctx.currentFrame.palettes.dmg.obp1  = ctx.registers.obp1;

          for (i32 y = 0; y < HEIGHT; ++y)
          {
            for (i32 x = 0; x < WIDTH; ++x)
            {
              i32 index = y * WIDTH + x;
              ctx.currentFrame.pixels[index].bits.colorIndex  = (u8) (((x + y + frameCounter) / 8) % 4);
              ctx.currentFrame.pixels[index].bits.paletteId   = 0;
              ctx.currentFrame.pixels[index].bits.layer       = 0;
            }
          }

          platformGetContext()->rendering.renderFrame(&ctx.currentFrame);
        }
        else ctx.mode = PPU_MODE_OAM_SCAN;
      }
      break;

    case PPU_MODE_VBLANK:
      if (ctx.dotCount >= 456)
      {
        ctx.dotCount -= 456;
        ctx.registers.ly++;

        if (ctx.registers.ly > 153)
        {
          ctx.registers.ly  = 0;
          ctx.mode          = PPU_MODE_OAM_SCAN;
        }
      }
      break;
  }
}
