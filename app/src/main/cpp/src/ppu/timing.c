#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cpu/interrupts.h>

void ppuTick(void)
{
  PpuContext* ctx = ppuGetContext();
  ppuStepOamDma();

  if ((ctx->registers.lcdc & LCDC_ENABLE_MASK) == 0)
  {
    ctx->dotCount++;
    if (ctx->dotCount >= PPU_DOTS_PER_FRAME)
    {
        ctx->dotCount = 0;
        ctx->frameReady = true; 
    }

    ctx->registers.ly        = 0;
    ctx->windowLineCounter   = 0; // - - - Reset window line
    ctx->mode                = PPU_MODE_HBLANK;
    ctx->registers.stat     &= ~STAT_MODE_BITS_MASK;
    return;
  }

  ctx->dotCount++;

  if (ctx->registers.ly < LY_VBLANK_START)
  {
    if (ctx->dotCount == 1)
    {
      ctx->mode             = PPU_MODE_OAM_SCAN;
      ctx->windowTriggered  = false;
      
      ppuResetFetcher();
      ppuResetFifos(); 
      ppuExecuteOamScan();
    }
    else if (ctx->dotCount < DOT_OAM_SCAN) 
    { ctx->mode = PPU_MODE_OAM_SCAN; }
    else if (ctx->dotCount < DOTS_HBLANK_START)
    {
      ctx->mode = PPU_MODE_DRAWING;
      ppuStepPixelFetcher();
      ppuStepPixelMixer(); 
    }
    else 
    {
      ctx->mode = PPU_MODE_HBLANK;
    }
  }
  else ctx->mode = PPU_MODE_VBLANK;

  if (ctx->dotCount >= PPU_DOTS_PER_SCANLINE)
  {
    ctx->dotCount = 0;

    if (ctx->windowTriggered) ctx->windowLineCounter++;
    
    ctx->registers.ly++;

    if (ctx->registers.ly >= LY_PER_FRAME) 
    {
      ctx->registers.ly        = 0;
      ctx->windowLineCounter   = 0; // - - - Reset window line for next frame
      ctx->mode                = PPU_MODE_OAM_SCAN;
      ctx->frameReady          = true;
    }
  }

  ctx->registers.stat &= ~STAT_MODE_BITS_MASK;
  ctx->registers.stat |= (ctx->mode & STAT_MODE_BITS_MASK);

  ppuUpdateInterrupts();
}
