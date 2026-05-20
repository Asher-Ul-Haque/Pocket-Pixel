#include <ppu/ppu.h>
#include <cpu/interrupts.h>

bool ppuIsLcdEnabled(void)
{
  const PpuContext* ctx = ppuGetContext();
  return (ctx->registers.lcdc & LCDC_ENABLE_MASK) != 0;
}

void ppuUpdateStatLycFlag(void)
{
  PpuContext* ctx = ppuGetContext();
  const bool lycMatch = ctx->registers.ly == ctx->registers.lyc;

  if (lycMatch) ctx->registers.stat |= STAT_LYC_EQUALS_MASK;
  else          ctx->registers.stat &= (u8)(~STAT_LYC_EQUALS_MASK);
}

void ppuHandleLycCompareEdge(bool previousMatch, bool currentMatch)
{
  PpuContext* ctx = ppuGetContext();

  if (!previousMatch &&
      currentMatch &&
      (ctx->registers.stat & STAT_LYC_INT_MASK))
  {
    cpuRequestInterrupt(CPU_INT_LCD);
  }
}

void ppuHandleModeInterrupt(PpuMode mode)
{
  PpuContext* ctx = ppuGetContext();

  switch (mode)
  {
    case PPU_MODE_HBLANK:
      if (ctx->registers.stat & STAT_HBLANK_INT_MASK) cpuRequestInterrupt(CPU_INT_LCD);
      break;
    case PPU_MODE_VBLANK:
      if (ctx->registers.stat & STAT_VBLANK_INT_MASK) cpuRequestInterrupt(CPU_INT_LCD);
      break;
    case PPU_MODE_OAM_SCAN:
      if (ctx->registers.stat & STAT_OAM_INT_MASK) cpuRequestInterrupt(CPU_INT_LCD);
      break;
    case PPU_MODE_DRAWING:
    default:
      break;
  }
}

void ppuSetMode(PpuMode mode)
{
  PpuContext* ctx = ppuGetContext();
  if (ctx->mode == mode) return;

  ctx->mode = mode;
  ctx->registers.stat &= (u8) ~STAT_MODE_BITS_MASK;
  ctx->registers.stat |= (u8) mode;

  if (ppuIsLcdEnabled()) ppuHandleModeInterrupt(mode);
}

void ppuHandleLcdStateChange(u8 previousLcdc, u8 newLcdc)
{
  PpuContext* ctx = ppuGetContext();
  const bool oldEnabled = (previousLcdc & LCDC_ENABLE_MASK) != 0;
  const bool newEnabled = (newLcdc & LCDC_ENABLE_MASK) != 0;

  if (oldEnabled == newEnabled) return;

  if (!newEnabled)
  {
    ctx->dotCount      = 0;
    ctx->registers.ly  = 0;
    ctx->frameReady    = false;
    ctx->mode = PPU_MODE_HBLANK;
    ctx->registers.stat &= (u8) ~STAT_MODE_BITS_MASK;
    ctx->registers.stat |= (u8) PPU_MODE_HBLANK;
    ppuUpdateStatLycFlag();
    return;
  }

  ctx->dotCount      = 0;
  ctx->registers.ly  = 0;
  ctx->frameReady    = false;
  ppuSetMode(PPU_MODE_OAM_SCAN);

  ppuUpdateStatLycFlag();
}
