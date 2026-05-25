#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cpu/interrupts.h>

static u16 ppuCalculateMode3Duration(void)
{
  PpuContext* ctx = ppuGetContext();

  u16 duration  = DOTS_DRAWING;

  // - - - 1. Background scrolling penalty
  duration      += (ctx->registers.scx & TILE_PIXEL_MASK);

  // - - - 2. Window actuvation penalty
  bool windowEnabled = (ctx->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;
  if (windowEnabled                          &&
      ctx->registers.ly >= ctx->registers.wy &&
      ctx->registers.wx <= WINDOW_WX_MAX)
  { duration += BG_SCROLLING_PENALTY; }

  // - - - 3. Sprite fetch penalty
  duration += ppuGetSpriteTimingPenalties();

  // - - - 4. Clamp max 
  if (duration > DOTS_DRAWING_MAX) duration = DOTS_DRAWING_MAX;
  return duration;
}

void ppuTick(void)
{
  PpuContext* ctx = ppuGetContext();
  ppuStepOamDma();

  // - - - Clearing LCDC Bit 7 immediately disables the PPU, forcing resets
  if ((ctx->registers.lcdc & LCDC_ENABLE_MASK) == 0)
  {
    ctx->dotCount        = 0;
    ctx->registers.ly    = 0;
    ctx->mode            = PPU_MODE_HBLANK;
    ctx->registers.stat &= ~STAT_MODE_BITS_MASK;
    return;
  }

  ctx->dotCount++;

  // - - - Vertical line lifecycle evaluation 
  if (ctx->registers.ly < LY_VBLANK_START)
  {
    // - - - Dot 1: run the physical hardware OAM search engine at the immediate onset of Mode 2 
    if (ctx->dotCount == PIXEL_COLOR_MASK)
    {
      ctx->mode             = PPU_MODE_OAM_SCAN;
      ctx->windowTriggered  = false;
      ppuResetFetcher();
      ppuExecuteOamScan();
    }

    // - - - Dots 0-79: Mode 2 (OAM Scan)
    else if (ctx->dotCount < DOT_OAM_SCAN) 
    { ctx->mode = PPU_MODE_OAM_SCAN; }

    // - - - Dot 80: Trigger mode 3 and calculate dynamic phase bounds
    else if (ctx->dotCount == DOT_OAM_SCAN)
    {
      ctx->mode           = PPU_MODE_DRAWING;
      ctx->mode3Duration  = ppuCalculateMode3Duration();
    }

    // - - - valuate Mode 3 running pipeline vs. H-Blank onset transition point
    else if (ctx->dotCount >= (u32)(DOT_OAM_SCAN + ctx->mode3Duration))
    {
      if (ctx->mode != PPU_MODE_HBLANK)
      {
        ctx->mode = PPU_MODE_HBLANK;
        ppuCheckHblankDma();
      }
    }

    if (ctx->mode == PPU_MODE_DRAWING) ppuStepPixelFetcher();
  }

  // - - - Vertical blank scanlines (144-153)
  else ctx->mode = PPU_MODE_VBLANK;

  // - - - Horizontal Scanline end boundary detection (456 dots per line)
  if (ctx->dotCount >= PPU_DOTS_PER_SCANLINE)
  {
    ctx->dotCount = 0;

    // - - - If the window was visible, advance its internal row index 
    if (ctx->windowTriggered) ctx->windowLineCounter++;
    
    ctx->registers.ly++;

    // - - - Frame is officially complete when we exceed the absolute max line (153)
    if (ctx->registers.ly >= LY_PER_FRAME) 
    {
      ctx->registers.ly = 0;
      ctx->mode         = PPU_MODE_OAM_SCAN;
      ctx->frameReady   = true;
    }
  }

  // - - - Synchronize registers cores with hardware state 
  ctx->registers.stat &= ~STAT_MODE_BITS_MASK;
  ctx->registers.stat |= (ctx->mode & STAT_MODE_BITS_MASK);

  ppuUpdateInterrupts();
}
