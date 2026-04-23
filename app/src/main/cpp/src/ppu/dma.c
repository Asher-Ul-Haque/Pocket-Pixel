#include <ppu/dma.h>
#include <bus.h>
#include <ppu/ppu.h>
#include <string.h>

static DmaContext ctx;
DmaContext* dmaGetContext(void) 
{ return &ctx; }

void dmaInit(void) 
{ memset(&ctx, 0, sizeof(ctx)); }

void dmaStart(u8 VALUE) 
{
  ctx.active     = true;
  ctx.sourceAddr = (u16)VALUE << 8;
  ctx.byteIndex  = 0;
  ctx.delay      = DMA_DELAY_CYCLES; // - - - Initial delay cycles
}

bool dmaIsActive(void) 
{ return ctx.active; }

void dmaTick(void) 
{
  if (!ctx.active) return;

  if (ctx.delay > 0) 
  {
    ctx.delay--;
    return;
  }

  // - - - Perform the transfer: 1 byte per M-cycle
  u8 data = busRead(ctx.sourceAddr + ctx.byteIndex);
  ppuOAMWrite(DMA_OFFSET + ctx.byteIndex, data);

  ctx.byteIndex++;

  if (ctx.byteIndex >= DMA_TRANSFER_CYCLES) 
  {
    ctx.active = false;
  }
}
