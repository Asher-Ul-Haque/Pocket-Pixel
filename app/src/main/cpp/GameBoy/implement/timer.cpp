#include "../include/timer.h"
#include "../include/interrupt.h"

static TimerContext ctx = {0};

TimerContext* timerGetContext() 
{ return &ctx; }

void timerInit() 
{ ctx.div = 0xAC00; }

void timerTick() 
{
  bool timerUpdate = false;
  u16  prevDiv     = ctx.div;
  ctx.div++;

  switch(ctx.tac & (0b11)) 
  {
    case 0b00:
      timerUpdate = (prevDiv & (1 << 9)) && (!(ctx.div & (1 << 9)));
      break;
    case 0b01:
      timerUpdate = (prevDiv & (1 << 3)) && (!(ctx.div & (1 << 3)));
      break;
    case 0b10:
      timerUpdate = (prevDiv & (1 << 5)) && (!(ctx.div & (1 << 5)));
      break;
    case 0b11:
      timerUpdate = (prevDiv & (1 << 7)) && (!(ctx.div & (1 << 7)));
      break;
  }

  if (timerUpdate && ctx.tac & (1 << 2)) 
  {
    ctx.tima++;

    if (ctx.tima == 0xFF) 
    {
      ctx.tima = ctx.tma;
      cpuRequestInterrupt(IT_TIMER);
    }
  }
}

void timerWrite(u16 ADDRESS, u8 VALUE) 
{
  switch(ADDRESS) 
  {
    // - - - DIV
    case 0xFF04:
      ctx.div = 0;
      break;

    // - - - TIMA
    case 0xFF05:
      ctx.tima = VALUE;
      break;

    // - - - TMA
    case 0xFF06:
      ctx.tma = VALUE;
      break;

    // - - - TAC
    case 0xFF07:
      ctx.tac = VALUE;
      break;
  }
}

u8 timerRead(u16 ADDRESS) 
{
  switch(ADDRESS) 
  {
    case 0xFF04: return ctx.div >> 8;
    case 0xFF05: return ctx.tima;
    case 0xFF06: return ctx.tma;
    case 0xFF07: return ctx.tac;
  }
}
