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

  ppuHandleModeInterrupt(ctx->mode);
}

void ppuHandleLycCompareEdge(bool PREVIOUS_MATCH, bool CURRENT_MATCH)
{
  (void) PREVIOUS_MATCH;
  (void) CURRENT_MATCH;
}

void ppuHandleModeInterrupt(PpuMode MODE)
{
  PpuContext* ctx = ppuGetContext();

  bool hblankInt  = (ctx->registers.stat & STAT_HBLANK_INT_MASK) && (MODE == PPU_MODE_HBLANK);
  bool vblankInt  = (ctx->registers.stat & STAT_VBLANK_INT_MASK) && (MODE == PPU_MODE_VBLANK);
  bool oamScanInt = (ctx->registers.stat & STAT_OAM_INT_MASK)    && (MODE == PPU_MODE_OAM_SCAN);
  bool lycInt     = (ctx->registers.stat & STAT_LYC_INT_MASK)    && (ctx->registers.ly == ctx->registers.lyc);

  // - - - The final hardware line is high if ANY source condition matches
  bool currentLineState = hblankInt || vblankInt || oamScanInt || lycInt;

  // - - - An interrupt can ONLY be sent to the CPU on a RISING EDGE (0 to 1 transition)
  if (!ctx->statLineState && currentLineState)
  {
    if (ppuIsLcdEnabled())
    {
      cpuRequestInterrupt(CPU_INT_LCD);
    }
  }

  // - - - Preserve the state of the line for subsequent transitions
  ctx->statLineState = currentLineState;
}

void ppuSetMode(PpuMode MODE)
{
  PpuContext* ctx = ppuGetContext();
  if (ctx->mode == MODE) return;

  ctx->mode = MODE;
  ctx->registers.stat &= (u8) ~STAT_MODE_BITS_MASK;
  ctx->registers.stat |= (u8) MODE;

  ppuHandleModeInterrupt(MODE);
}

void ppuHandleLcdStateChange(u8 PREVIOUS_LCDC, u8 NEW_LCDC)
{
  PpuContext* ctx = ppuGetContext();
  const bool oldEnabled = (PREVIOUS_LCDC & LCDC_ENABLE_MASK) != 0;
  const bool newEnabled = (NEW_LCDC & LCDC_ENABLE_MASK) != 0;

  if (oldEnabled == newEnabled) return;

  if (!newEnabled)
  {
    ctx->dotCount        = 0;
    ctx->registers.ly    = 0;
    for (u32 i = 0; i < (WIDTH * HEIGHT); ++i)
    {
      ctx->currentFrame.resolvedColor[i] = 0x7FFF;
      ctx->currentFrame.pixels[i].raw = 0;
    }
    ctx->frameReady      = true;
    ctx->mode            = PPU_MODE_HBLANK;
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
