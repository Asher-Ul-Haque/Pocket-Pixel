#include <bus.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <cpu/interrupts.h>
#include <string.h>

static PpuContext ctx;

PpuContext* ppuGetContext(void) { return &ctx; }

void ppuInit(void)
{
  memset(&ctx, 0, sizeof(PpuContext));

  bool isDMG = cartridgeGetContext()->mode == MODE_DMG_GAMEBOY;

  // - - - Standard Shared Bootrom Exit States - - -
  ctx.registers.lcdc = BOOT_LCDC;
  ctx.registers.stat = BOOT_STAT;
  ctx.registers.bgp  = BOOT_BGP;
  ctx.registers.obp0 = BOOT_OBP0;
  ctx.registers.obp1 = BOOT_OBP1;

  // - - - Architecture Specific Initializations - - -
  if (isDMG)
  {
    ctx.registers.ly = 0;
  }
  else
  {
    // - - - CGB specific register defaults
    ctx.registers.vbk  = DEFAULT_VRAM_BANK;
    ctx.registers.bgpi = 0;
    ctx.registers.obpi = 0;
    ctx.registers.ly   = 0;
  }

  // - - - Internal State Machine Defaults - - -
  ctx.mode = PPU_MODE_VBLANK;
  ctx.dotCount = 0;
  
  // - - - Flush internal queue memory - - -
  ppuResetFifos();
  ppuResetFetcher();
}

void ppuUpdateInterrupts(void)
{
  PpuContext* ctx = ppuGetContext();

  // - - - If the LCD is turned off, the internal signal is 0
  if ((ctx->registers.lcdc & LCDC_ENABLE_MASK) == 0)
  {
    ctx->statLineState = false;
    return;
  }

  // - - - 1. Evaluate the continuous LYC == LY status bit
  bool lycMatch = (ctx->registers.ly == ctx->registers.lyc);
  
  if (lycMatch)  ctx->registers.stat |= STAT_LYC_EQUALS_MASK; 
  else           ctx->registers.stat &= ~STAT_LYC_EQUALS_MASK; 

  // - - -  2. Gather individual interrupt source configurations from the STAT register select bits
  bool lycInterruptEnabled   = (ctx->registers.stat & STAT_LYC_INT_MASK)    != 0; // Bit 6
  bool mode2InterruptEnabled = (ctx->registers.stat & STAT_OAM_INT_MASK)    != 0; // Bit 5
  bool mode1InterruptEnabled = (ctx->registers.stat & STAT_VBLANK_INT_MASK) != 0; // Bit 4
  bool mode0InterruptEnabled = (ctx->registers.stat & STAT_HBLANK_INT_MASK) != 0; // Bit 3

  // - - - 3. Compute active hardware input signal values fed into the single OR gate
  bool lycCondition   = lycMatch && lycInterruptEnabled;
  bool mode2Condition = (ctx->mode == PPU_MODE_OAM_SCAN) && mode2InterruptEnabled;
  bool mode1Condition = (ctx->mode == PPU_MODE_VBLANK)   && mode1InterruptEnabled;
  bool mode0Condition = (ctx->mode == PPU_MODE_HBLANK)   && mode0InterruptEnabled;

  // - - - The conditions are routed to a single, shared OR gate line
  bool currentHardwareLine = lycCondition || mode2Condition || mode1Condition || mode0Condition;

  // - - - The STAT IRQ triggers when this combined signal hits a 0->1 rising edge
  if (!ctx->statLineState && currentHardwareLine)
  {
    cpuRequestInterrupt(CPU_INT_LCD); 
  }

  // - - - V-Blank Hardware Intercept: Trigger standard V-Blank interrupt line pulse the exact dot cycle we cross the visible threshold boundary into line 144
  if (ctx->registers.ly == LY_VBLANK_START && ctx->dotCount == 0)
  {
    cpuRequestInterrupt(CPU_INT_VBLANK);
  }

  // - - - Preserve the current state of the physical wire for subsequent clock cycle evaluations
  ctx->statLineState = currentHardwareLine;
}

void ppuPushPixelToScreen(u8 SCREEN_X, u8 SCREEN_Y, u16 RGB_555)
{
  if (SCREEN_X < WIDTH && SCREEN_Y < HEIGHT)
  {  ctx.frameBuffer.pixels[SCREEN_Y][SCREEN_X] = RGB_555;  }
}
