#include "../include/io.h"
#include "../include/common.h"
#include "../include/timer.h"
#include "../include/cpu.h"
#include "../include/lcd.h"
#include "../include/io.h"

static char serialData[2];

u8 ioRead(u16 ADDRESS)
{
  if (ADDRESS == 0xFF01)                return serialData[0];
  if (ADDRESS == 0xFF02)                return serialData[1];
  if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) return timerRead(ADDRESS);
  if (ADDRESS == 0xFF0F)                return cpuGetInterruptFlags();
  if (BETWEEN(ADDRESS, 0xFF40, 0xFF4B)) return lcdRead(ADDRESS);

  FORGE_LOG_ERROR("UNSUPPORTED bus_read(%04X)\n", ADDRESS);
  return 0;
}

void ioWrite(u16 ADDRESS, u8 VALUE) 
{
  if (ADDRESS == 0xFF01)
  {
    serialData[0] = VALUE;
    return;
  }

  if (ADDRESS == 0xFF02) 
  {
    serialData[1] = VALUE;
    return;
  }

  if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) 
  {
    timerWrite(ADDRESS, VALUE);
    return;
  }
    
  if (ADDRESS == 0xFF0F) 
  {
    cpuSetInterruptFlags(VALUE);
    return;
  }
  
  if (BETWEEN(ADDRESS, 0xFF40, 0xFF4B))
  {
    lcdWrite(ADDRESS, VALUE);
    return;
  }

  FORGE_LOG_ERROR("UNSUPPORTED bus_write(%04X)\n", ADDRESS);
}
