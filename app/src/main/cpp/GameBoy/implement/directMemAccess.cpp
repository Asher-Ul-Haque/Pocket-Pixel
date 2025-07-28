#include "../include/directMemAccess.h"
#include "../include/bus.h"
#include "../include/ppu.h" 

typedef struct
{
  bool active;
  u8   byte;
  u8   value;
  u8   startDelay;
} DMAcontext;

static DMAcontext ctx;

void dmaStart(u8 START)
{
  ctx.active     = true;
  ctx.byte       = 0;
  ctx.startDelay = 2;
  ctx.value      = START;
}

void dmaTick()
{
  if (!ctx.active) return;
  if (ctx.startDelay)
  {
    ctx.startDelay--;
    return;
  }
  ppuOAMwrite(ctx.byte, busRead((ctx.value * 0x100) + ctx.byte));

  ctx.byte++;
  ctx.active = (ctx.byte < 0xA0);
}

bool dmaTransferring()
{ return ctx.active; }
