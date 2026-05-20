#include <ppu/ppu.h>

void ppuRenderFrame(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->currentFrame.palettes.dmg.bgp  = ctx->registers.bgp;
  ctx->currentFrame.palettes.dmg.obp0 = ctx->registers.obp0;
  ctx->currentFrame.palettes.dmg.obp1 = ctx->registers.obp1;

  ppuRenderBgLayer();
  ppuRenderWindowLayer();
  ppuRenderObjLayer();
}
