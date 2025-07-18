#include "../include/timer.h"
#include "../include/interrupt.h"

static TimerContext timerCTX = {0};

void timerInit()
{ timerCTX.div = 0xAC00; }

void timerTick()
{
  u16 prevDiv = timerCTX.div;
  timerCTX.div++;
  bool update = false;

  switch(timerCTX.tac & 0b11)
  {
    case 0b00 : 
      { 
        update = 
          (prevDiv       & (1 << 9)) &&
          !(timerCTX.div & (1 << 9));
        break;
      }
    case 0b01 : 
      { 
        update = 
          (prevDiv       & (1 << 3)) &&
          !(timerCTX.div & (1 << 3));
        break;
      }
    case 0b10 : 
      { 
        update = 
          (prevDiv       & (1 << 5)) &&
          !(timerCTX.div & (1 << 5));
        break;
      }
    case 0b11 : 
      { 
        update = 
          (prevDiv       & (1 << 7)) &&
          !(timerCTX.div & (1 << 7));
        break;
      }
  }

  if (update && timerCTX.tac & (1 << 2))
  {
    timerCTX.tima++;
    if (timerCTX.tima == 0xFF) 
    {
      timerCTX.tima = timerCTX.tma;
      cpuRequestInterrupt(INTRPT_TIMER);
    }
  }
}

void timerWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    case 0xFF04 : { timerCTX.div  = 0;     break; }
    case 0xFF05 : { timerCTX.tima = VALUE; break; }
    case 0xFF06 : { timerCTX.tma  = VALUE; break; }
    case 0xFF07 : { timerCTX.tac  = VALUE; break; }
  }
}

u8 timerRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    case 0xFF04 : return timerCTX.div >> 8;
    case 0xFF05 : return timerCTX.tima;
    case 0xFF06 : return timerCTX.tma;
    case 0xFF07 : return timerCTX.tac;
  }
  FORGE_LOG_ERROR("Cannot read outside timer range 0x%04X", ADDRESS);
  return 0;
}

TimerContext* timerGetContext()
{ return &timerCTX; }
