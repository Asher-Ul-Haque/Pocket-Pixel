#include "../include/emu.h"
#include "../include/emu.h"
#include "../include/emu.h"
#include "../include/timer.h"

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
  i32 n = CPU_CYCLES * 4;

  for (i32 i = 0; i < n; i++) 
  {
    ctx.ticks++;
    timer_tick();
  }
}
