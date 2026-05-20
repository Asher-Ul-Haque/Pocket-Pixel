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

void ppuHandleLycCompareEdge(bool PREVIOUS_MATCH, bool CURRENT_MATCH)
{
  PpuContext* ctx = ppuGetContext();

  if (!PREVIOUS_MATCH &&
      CURRENT_MATCH &&
      (ctx->registers.stat & STAT_LYC_INT_MASK))
  {
    cpuRequestInterrupt(CPU_INT_LCD);
  }
}

void ppuHandleModeInterrupt(PpuMode MODE)
{
  PpuContext* ctx = ppuGetContext();

  switch (MODE)
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

void ppuSetMode(PpuMode MODE)
{
  PpuContext* ctx = ppuGetContext();
  if (ctx->mode == MODE) return;

  ctx->mode = MODE;

  if (ppuIsLcdEnabled()) ppuHandleModeInterrupt(MODE);
}

void ppuHandleLcdStateChange(u8 PREVIOUS_LCDC, u8 NEW_LCDC)
{
  PpuContext* ctx = ppuGetContext();
  const bool oldEnabled = (PREVIOUS_LCDC & LCDC_ENABLE_MASK) != 0;
  const bool newEnabled = (NEW_LCDC & LCDC_ENABLE_MASK) != 0;

  if (oldEnabled == newEnabled) return;

  if (!newEnabled)
  {
    ctx->dotCount      = 0;
    ctx->registers.ly  = 0;
    ctx->frameReady    = false;
    ctx->mode          = PPU_MODE_HBLANK;
    ppuUpdateStatLycFlag();
    return;
  }

  ctx->dotCount      = 0;
  ctx->registers.ly  = 0;
  ctx->frameReady    = false;
  ctx->mode          = PPU_MODE_OAM_SCAN;

  ppuUpdateStatLycFlag();
  ppuHandleModeInterrupt(PPU_MODE_OAM_SCAN);
}
