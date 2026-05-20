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
  ctx.dotCount = 0;
  ctx.frameReady = false;

  // - - - default hardware register values on bootup 
  ctx.registers.lcdc = BOOT_LCDC;
  ctx.registers.stat = BOOT_STAT & (STAT_UNUSED_HIGH_BIT | STAT_WRITABLE_BITS_MASK);
  ctx.registers.bgp  = BOOT_BGP;
  ctx.registers.obp0 = BOOT_OBP0;
  ctx.registers.obp1 = BOOT_OBP1;

  ppuUpdateStatLycFlag();
}
