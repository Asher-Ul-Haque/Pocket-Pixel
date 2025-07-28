#include "../include/bus.h"
#include "../include/cartridge.h"
#include "../include/ram.h"
#include "../include/cpu.h"
#include "../include/ppu.h"
#include "../include/io.h"
#include "../include/directMemAccess.h"

/* How the bus addressing works
0x0000 - 0x3FFF : ROM Bank 0
0x4000 - 0x7FFF : ROM Bank 1 - Switchable
0x8000 - 0x97FF : CHR RAM (VRAM)
0x9800 - 0x9BFF : BG Map 1 (VRAM)
0x9C00 - 0x9FFF : BG Map 2 (VRAM)
0xA000 - 0xBFFF : Cartridge RAM
0xC000 - 0xCFFF : RAM Bank 0 (WRAM)
0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only (WRAM)
0xE000 - 0xFDFF : Reserved - Echo RAM (WRAM Echo)
0xFE00 - 0xFE9F : Object Attribute Memory (OAM)
0xFEA0 - 0xFEFF : Reserved - Unusable
0xFF00 - 0xFF7F : I/O Registers
0xFF80 - 0xFFFE : Zero Page (HRAM)
*/

// - - - 8 bit operations - - -
FORGE_API u8 busRead(u16 ADDRESS)
{
  if      (ADDRESS < 0x8000)  return cartridgeRead(ADDRESS);
  else if (ADDRESS < 0xA000)  return ppuVRAMread(ADDRESS); 
  else if (ADDRESS < 0xC000)  return cartridgeRead(ADDRESS);
  else if (ADDRESS < 0xE000)  return wramRead(ADDRESS);
  else if (ADDRESS < 0xFE00)  return 0xFF; 
  else if (ADDRESS < 0xFEA0)
  {
    if (dmaTransferring())  return 0xFF; 
    else                    return ppuOAMread(ADDRESS); 
  }
  else if (ADDRESS < 0xFF00)  return 0xFF; 
  else if (ADDRESS < 0xFF80)  return ioRead(ADDRESS);
  else if (ADDRESS == 0xFFFF) return cpuGetInterrupt(); 

  return hramRead(ADDRESS);
}

FORGE_API void busWrite(u16 ADDRESS, u8 VALUE)
{
  if      (ADDRESS < 0x8000)    { cartridgeWrite(ADDRESS, VALUE); }
  else if (ADDRESS < 0xA000)    { ppuVRAMwrite(ADDRESS, VALUE); } 
  else if (ADDRESS < 0xC000)    { cartridgeWrite(ADDRESS, VALUE); } 
  else if (ADDRESS < 0xE000)    { wramWrite(ADDRESS, VALUE); }
  else if (ADDRESS < 0xFE00)    { /* Echo RAM / Unused area, writes ignored */ }
  else if (ADDRESS < 0xFEA0)
  {
    if (dmaTransferring())  return; 
    else                    ppuOAMwrite(ADDRESS, VALUE);
  }
  else if (ADDRESS < 0xFF00)    { /* Unusable I/O area, writes ignored */ }
  else if (ADDRESS < 0xFF80)    { ioWrite(ADDRESS, VALUE); }
  else if (ADDRESS == 0xFFFF)   { cpuSetInterrupt(VALUE); } 
  else                          { hramWrite(ADDRESS, VALUE); } 
}


// - - - 16 bit operations - - -
FORGE_API u16 busRead16(u16 ADDRESS)
{
  u16 lo = busRead(ADDRESS);
  u16 hi = busRead(ADDRESS + 1);

  return lo | (hi << 8);
}

FORGE_API void busWrite16(u16 ADDRESS, u16 VALUE)
{
  busWrite(ADDRESS + 1, (VALUE >> 8) & 0xFF);
  busWrite(ADDRESS, VALUE & 0xFF);
}
