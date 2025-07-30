#include "../include/io.h"
#include "../include/gamepad.h"
#include "../include/timer.h"
#include "../include/cpu.h"
#include "../include/apu.h" 
#include "../include/ppu.h"  
#include "../include/common.h"
#include "../include/serial.h"


u8 ioRead(u16 ADDRESS)
{
  if (ADDRESS == 0xFF00)                return gamepadRead();
  if (BETWEEN(ADDRESS, 0xFF01, 0xFF02)) return serialRead(ADDRESS);
  if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) return timerRead(ADDRESS);
  if (ADDRESS == 0xFF0F)                return cpuGetInterruptFlags();
  if (BETWEEN(ADDRESS, 0xFF40, 0xFF4B)) return ppuRead(ADDRESS); 
  if (BETWEEN(ADDRESS, 0xFF10, 0xFF26) || (BETWEEN(ADDRESS, 0xFF30, 0xFF3F))) 
    return apuRead(ADDRESS);
   
  if (ADDRESS == 0xFF4F) return 0xFF; // - - - gmaeboy color 
  if (ADDRESS == 0xFF50) return 0xFF; // - - - boot rom  
  if (ADDRESS == 0xFF7F) return 0xFF; // - - - unused

  FORGE_LOG_ERROR("UNSUPPORTED ioRead(%04X)\n", ADDRESS);
  return 0;
}

void ioWrite(u16 ADDRESS, u8 VALUE) 
{
  if (ADDRESS == 0xFF00)
  {
    gamepadWrite(VALUE);
    return;
  }

  if (BETWEEN(ADDRESS, 0xFF01, 0xFF02)) 
  {
    serialWrite(ADDRESS, VALUE);
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
    ppuWrite(ADDRESS, VALUE); 
    return;
  }

  if (BETWEEN(ADDRESS, 0xFF10, 0xFF26) || (BETWEEN(ADDRESS, 0xFF30, 0xFF3F))) 
  { 
    apuWrite(ADDRESS, VALUE);
    return;
  }

  if (ADDRESS == 0xFF4F) return; // - - - gameboy color 
  if (ADDRESS == 0xFF50) return; // - - - boot rom  
  if (ADDRESS == 0xFF7F) return; // - - - unused io 

  FORGE_LOG_ERROR("UNSUPPORTED ioWrite(%04X) = %02X\n", ADDRESS, VALUE);
}
