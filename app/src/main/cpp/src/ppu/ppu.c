#include <bus.h>
#include <platform.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <string.h>

static PpuContext ctx;

PpuContext* ppuGetContext(void) { return &ctx; }

void ppuInit(void)
{
  memset(&ctx, 0, sizeof(PpuContext));
  ctx.mode = PPU_MODE_OAM_SCAN;

  // - - - default hardware register values on bootup 
  ctx.registers.lcdc = BOOT_LCDC;
  ctx.registers.stat = BOOT_STAT;
  ctx.registers.bgp  = BOOT_BGP;
  ctx.registers.obp0 = BOOT_OBP0;
  ctx.registers.obp1 = BOOT_OBP1;
}

void ppuTick(u32 DOTS)
{
  ctx.dotCount += DOTS;

  switch (ctx.mode)
  {
    case PPU_MODE_OAM_SCAN:
      if (ctx.dotCount >= DOT_OAM_SCAN)
      {
        ctx.dotCount -= DOT_OAM_SCAN;
        ctx.mode      = PPU_MODE_DRAWING;
      }
      break;

    case PPU_MODE_DRAWING:
      if (ctx.dotCount >= DOTS_DRAWING)
      {
        ctx.dotCount -= DOTS_DRAWING;
        ctx.mode      = PPU_MODE_HBLANK;
      }
      break;

    case PPU_MODE_HBLANK:
      if (ctx.dotCount >= DOTS_HBLANK)
      {
        ctx.dotCount -= DOTS_HBLANK;
        ctx.registers.ly++;

        if (ctx.registers.ly == LY_VBLANK_START)
        {
          ctx.mode = PPU_MODE_VBLANK;

          // - - - Verification
          ctx.testPatternFrame++;
          ctx.currentFrame.palettes.dmg.bgp   = ctx.registers.bgp;
          ctx.currentFrame.palettes.dmg.obp0  = ctx.registers.obp0;
          ctx.currentFrame.palettes.dmg.obp1  = ctx.registers.obp1;

          for (i32 y = 0; y < HEIGHT; ++y)
          {
            for (i32 x = 0; x < WIDTH; ++x)
            {
              i32 index = y * WIDTH + x;
              ctx.currentFrame.pixels[index].bits.colorIndex = (u8)(((x + y + ctx.testPatternFrame) / 8) % 4);
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
      if (ctx.dotCount >= DOTS_VBLANK)
      {
        ctx.dotCount -= DOTS_VBLANK;
        ctx.registers.ly++;

        if (ctx.registers.ly > LY_MAX)
        {
          ctx.registers.ly  = 0;
          ctx.mode          = PPU_MODE_OAM_SCAN;
        }
      }
      break;
  }
}
