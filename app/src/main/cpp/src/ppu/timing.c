#include <platform.h>
#include <ppu/ppu.h>
#include <cpu/interrupts.h>

static void ppuStepScanlineDot(void)
{
  PpuContext* ctx = ppuGetContext();
  ctx->dotCount++;

  if (ctx->registers.ly < LY_VBLANK_START)
  {
    if (ctx->dotCount == DOTS_TRANSFER_START) ppuSetMode(PPU_MODE_DRAWING);
    if (ctx->dotCount == DOTS_HBLANK_START)   ppuSetMode(PPU_MODE_HBLANK);
  }

  if (ctx->dotCount < PPU_DOTS_PER_SCANLINE) return;

  const bool previousMatch = ctx->registers.ly == ctx->registers.lyc;
  ctx->dotCount = 0;
  ctx->registers.ly++;
  if (ctx->registers.ly >= LY_PER_FRAME) ctx->registers.ly = 0;

  const bool currentMatch = ctx->registers.ly == ctx->registers.lyc;
  ppuUpdateStatLycFlag();
  ppuHandleLycCompareEdge(previousMatch, currentMatch);

  if (ctx->registers.ly == LY_VBLANK_START)
  {
    ppuSetMode(PPU_MODE_VBLANK);
    cpuRequestInterrupt(CPU_INT_VBLANK);
    ppuRenderFrame();
    platformGetContext()->rendering.renderFrame(&ctx->currentFrame);
    ctx->frameReady = true;
    return;
  }

  if (ctx->registers.ly < LY_VBLANK_START)
  {
    ppuSetMode(PPU_MODE_OAM_SCAN);
    return;
  }

  ppuSetMode(PPU_MODE_VBLANK);
}

void ppuTick(u32 dots)
{
  PpuContext* ctx = ppuGetContext();
  ctx->frameReady = false;

  if (!ppuIsLcdEnabled()) return;

  for (u32 dot = 0; dot < dots; ++dot)
  {
    ppuStepScanlineDot();
  }
}
