#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <ppu/oam.h>
#include <ppu/dma.h>
#include <ppu/ppuRegisters.h>
#include <cpu/interrupts.h>

/**
 * @file ppu.c
 * @brief Main PPU logic, including mode transitions, LY/LYC handling, and interrupt triggering.
 * This file orchestrates the overall PPU behavior and timing, coordinating the fetcher, OAM search, and pixel pushing to the screen.
 * It also handles the critical timing of mode transitions and the triggering of STAT interrupts based on LY/LYC comparisons and mode changes.
*/

static PpuContext ctx;
PpuContext* ppuGetContext(void) { return &ctx; }

void ppuInit(void)
{
  memset(&ctx, 0, sizeof(ctx));

  // - - - Hardware defaults
  ctx.lcdc = DEFAULT_LCDC;
  ctx.stat = DEFAULT_STAT;
  STAT_SET_MODE(&ctx, PPU_MODE_OAM);
}

static void handleModeTransitions(void)
{
  PpuMode currentMode = STAT_GET_MODE(&ctx);

  switch (currentMode)
  {
    case PPU_MODE_OAM:
      {
        // - - - Mode 2 always takes 80 cycles
        if (ctx.dotClock >= T_CYCLES_MODE_2)
        {
          STAT_SET_MODE(&ctx, PPU_MODE_DRAW);

          // - - - Reset pipeline for Mode 3
          ctx.pixelsPushed      = 0;
          ctx.windowTriggered   = false;
          ctx.fetcher.state     = FETCH_GET_TILE;
          ctx.fetcher.xOffset   = 0;
          ctx.bgFifo.size       = 0;
          ppuOamResetSearch();
        }
        break;
      }

    case PPU_MODE_DRAW:
      {
        // - - - Logic handled by pipeline
        if (ctx.pixelsPushed >= SCREEN_PIXELS_X)
        {
          STAT_SET_MODE(&ctx, PPU_MODE_HBLANK);
          if (STAT_MODE0_INT(&ctx)) cpuRequestInterrupt(CPU_INT_LCD);
        }
        break;
      }

    case PPU_MODE_HBLANK:
      {
        // = = = A full line (MOdes 2 + 3 + 0) takes 456 cycles
        if (ctx.dotClock >= T_CYCLES_SCANLINE)
        {
          ctx.dotClock = 0;
          ctx.ly++;
      
          if (ctx.ly >= SCREEN_HEIGHT)
          {
            // - - - Entering V-Blank
            STAT_SET_MODE(&ctx, PPU_MODE_VBLANK);
            cpuRequestInterrupt(CPU_INT_VBLANK);
        
            if (STAT_MODE1_INT(&ctx)) cpuRequestInterrupt(CPU_INT_LCD);
    
            // - - - Reset window line counter for next frame 
            ctx.windowLineCounter = 0;
          }
          else 
          {
            STAT_SET_MODE(&ctx, PPU_MODE_OAM);
            if (STAT_MODE2_INT(&ctx)) cpuRequestInterrupt(CPU_INT_LCD);
          }
        }
        break;
      }

    case PPU_MODE_VBLANK:
      {
        if (ctx.dotClock >= T_CYCLES_SCANLINE)
        {
          ctx.dotClock = 0;
          ctx.ly++;

          // - - - Line 153 logic 
          if (ctx.ly > VBLANK_END_LINE)
          {
            ctx.ly = 0;
            STAT_SET_MODE(&ctx, PPU_MODE_OAM);
            if (STAT_MODE2_INT(&ctx)) cpuRequestInterrupt(CPU_INT_LCD);
          }
        }
        break;
      }
  }
}

void ppuStepTCycle(void)
{
  // - - - if LCD is disabled, PPU does not advance
  if (!LCDC_ENABLED(&ctx)) return;

  // - - - advances cycles 
  ctx.dotClock++;
  ctx.frameClock++;

  // - - - 1. Precise LY=LYC Comparison 
  if (ctx.ly == ctx.lyc)
  {
    STAT_SET_LYC_FLAG(&ctx);
    if (STAT_LYC_INT(&ctx)) cpuRequestInterrupt(CPU_INT_LCD);
  }
  else
  {
    STAT_CLEAR_LYC_FLAG(&ctx);
  }

  // - - - 2. Delegate sub logic based on current mode and timing
  PpuMode mode = STAT_GET_MODE(&ctx);
  switch (mode)
  {
    case PPU_MODE_OAM:   ppuOamSearchTick(); break;
    case PPU_MODE_DRAW:  ppuPipelineTick(); break;
    default: break; // - - - Mode transitions and V-Blank logic handled separately
  }

  // - - - 3. Handle mode transitions and V-Blank timing
  handleModeTransitions();
}

void ppuStepMCycle(void)
{
  for (u8 i = 0; i < 4; i++)   ppuStepTCycle();
}
