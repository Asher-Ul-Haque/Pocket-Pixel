#include "../include/io.h"
#include "../include/common.h"
#include "../include/timer.h"
#include "../include/cpu.h"
#include "../include/directMemAccess.h"

static char serialData[2];
u8 ly = 0;

u8 ioRead(u16 ADDRESS)
{
  if (ADDRESS == 0xFF01)                return serialData[0];
  if (ADDRESS == 0xFF02)                return serialData[1];
  if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) return timerRead(ADDRESS);
  if (ADDRESS == 0xFF0F)                return cpuGetInterruptFlags();
  if (ADDRESS == 0xFF44)                return ly++;

  FORGE_LOG_ERROR("UNSUPPORTED bus_read(%04X)\n", ADDRESS);
  return 0;
}

void ioWrite(u16 ADDRESS, u8 VALUE) 
{
  FORGE_LOG_WARNING("Recieved io writing : %04X value : $02X", ADDRESS, VALUE);
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
  
  if (ADDRESS == 0xFF46)
  {
    dmaStart(VALUE);
    FORGE_LOG_TRACE("Starting dma with %04X", VALUE);
  }

  FORGE_LOG_ERROR("UNSUPPORTED bus_write(%04X)\n", ADDRESS);
}
