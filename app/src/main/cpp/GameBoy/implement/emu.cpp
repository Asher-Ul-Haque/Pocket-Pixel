#include "../include/emu.h"
#include "../include/emu.h"
#include "../include/emu.h"
#include "../include/timer.h"
#include "../include/directMemAccess.h"
#include "../include/ppu.h"
#include "../include/apu.h"

/* 
  Emu components:

  |Cart|
  |CPU|
  |Address Bus|
  |PPU|
  |Timer|

*/

static EMUcontext ctx;

EMUcontext* emuGetContext() 
{ return &ctx; }

void emuCycles(i32 CPU_CYCLES) 
{
  for (int i = 0; i < CPU_CYCLES; ++i)
  {
    for (int n = 0; n < 4; ++n)
    {
      ctx.ticks++;
      timerTick();
      ppuTick();
      apuUpdate(1);
    }
    dmaTick();
  }
}
