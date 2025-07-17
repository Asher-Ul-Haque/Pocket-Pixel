#include "../include/io.h"
#include "../include/timer.h"
#include "../include/cpu.h"


static u8 serialData[2];

u8 IOread(u16 ADDRESS)
{
  if (ADDRESS == 0xFF01)   return serialData[0];
  if (ADDRESS == 0xFF02)   return serialData[1];
  if (ADDRESS == 0xFF0F)   return cpuGetInterrupt();
  if (ADDRESS >= 0xFF04 && ADDRESS <= 0xFF07)
  { return timerRead(ADDRESS); }

  TODO_COMMENT("Implement other io ports");
  return -1;
}

void IOwrite(u16 ADDRESS, u8 VALUE)
{  
  if (ADDRESS == 0xFF01)   { serialData[0] = VALUE;  return; }
  if (ADDRESS == 0xFF02)   { serialData[1] = VALUE;  return; }
  if (ADDRESS == 0xFF0F)   { cpuSetInterrupt(VALUE); return;}
  if (ADDRESS >= 0xFF04 && ADDRESS <= 0xFF07)
  { 
    timerWrite(ADDRESS, VALUE); 
    return;
  }

  TODO_COMMENT("Implement other io ports");
}
