#include <bus.h>
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
  ctx.registers.stat = (BOOT_STAT & (STAT_UNUSED_HIGH_BIT | STAT_WRITABLE_BITS_MASK)) | (u8) ctx.mode;
  ctx.registers.bgp  = BOOT_BGP;
  ctx.registers.obp0 = BOOT_OBP0;
  ctx.registers.obp1 = BOOT_OBP1;

  for (u32 i = 0; i < (CGB_PALETTE_COLOR_COUNT * CGB_PALETTE_COUNT); ++i)
  {
    ctx.currentFrame.palettes.cgb.bg[i]  = 0x7FFF;
    ctx.currentFrame.palettes.cgb.obj[i] = 0x7FFF;
  }
  for (u32 i = 0; i < PALETTE_RAM_SIZE; i += 2)
  {
    ctx.bgPaletteRam[i]     = 0xFF;
    ctx.bgPaletteRam[i + 1] = 0x7F;
    ctx.objPaletteRam[i]     = 0xFF;
    ctx.objPaletteRam[i + 1] = 0x7F;
  }
  for (u32 i = 0; i < (WIDTH * HEIGHT); ++i)
  {
    ctx.currentFrame.resolvedColor[i] = 0x7FFF;
  }

  ppuUpdateStatLycFlag();
}
