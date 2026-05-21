#include <platform.h>
#include <ppu/ppu.h>
#include <cpu/interrupts.h>

static void ppuStepScanlineDot(void)
{
  PpuContext* ctx = ppuGetContext();
  ctx->dotCount++;

  if (ctx->registers.ly < LY_VBLANK_START)
  {
    if (ctx->dotCount == DOTS_TRANSFER_START) 
    {
      ppuSetMode(PPU_MODE_DRAWING);
    }
    
    // FIX: Optimized timing calibration for Mooneye alignment suites.
    // We scale the drawing duration strictly using the SCX fine scroll alignment penalty.
    u32 mode3Duration = 172 + (ctx->registers.scx & 0x07);
    u32 dynamicHblankStart = DOTS_TRANSFER_START + mode3Duration;

    if (ctx->dotCount == dynamicHblankStart)
    {
      ppuSetMode(PPU_MODE_HBLANK);
      ppuRenderScanline(ctx->registers.ly);
    }
  }

  if (ctx->dotCount < PPU_DOTS_PER_SCANLINE) return;

  ctx->dotCount = 0;
  ctx->registers.ly++;
  if (ctx->registers.ly >= LY_PER_FRAME) ctx->registers.ly = 0;

  ppuUpdateStatLycFlag();
  
  if (ctx->registers.ly == LY_VBLANK_START)
  {
    ppuSetMode(PPU_MODE_VBLANK);
    cpuRequestInterrupt(CPU_INT_VBLANK);
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

void ppuTick(void)
{
  if (!ppuIsLcdEnabled()) return;

  PpuContext* ctx = ppuGetContext();

  // BITWISE SPEED EVALUATION:
  // Check bit 7 (0x80) of KEY1. If set, we are in Double Speed mode.
  // Double Speed = 2 dots per M-cycle. Normal Speed = 4 dots per M-cycle.
  u32 dotsToTick = (ctx->registers.doubleSpeed & 0x80) ? 2 : 4;

  for (u32 dot = 0; dot < dotsToTick; ++dot)
  {
    ppuStepScanlineDot();
  }
}
