#include "bus.h"
#include "cartridge.h"
#include "../../ForgeLibrary/include/asserts.h"

/* 
MEMORY MAP OF THE GAME BOY

  Cartridge 
  * 0x0000 - 0x3FFF : ROM Bank 0
  * 0x4000 - 0x7FFF : ROM Bank 1 (Switchable)
  
  Video Ram
  * 0x8000 - 0x97FF : CHR RAM 
  * 0x9800 - 0x9BFF : BG Map 1
  * 0x9C00 - 0x9FFF : BG Map 2

  * 0xA000 - 0xBFFF : Cartridge RAM 
  
  * 0xC000 - 0xCFFF : RAM Bank 0

  * 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
  
  Reserved 
  * 0xE000 - 0xFDFF : Reserved - Echo RAM 
  * 0xFE00 - 0xFE9F : Object Attribute Memory 
  * 0xFEA0 - 0xFEFF : Reserved - Unusable 

  * 0xFF00 - 0xFF7F : I/O registers

  * 0xFF80 - 0XFFFE : Zero Page

  * 0xFFFF          : Interrupt
*/

u8 busRead(u16 ADDRESS)
{
  if (ADDRESS < 0x8000) return cartridgeRead(ADDRESS);
  //TODO_COMMENT("We are doing only ROM reading for now");
  return 0;
}

void busWrite(u16 ADDRESS, u8 VALUE)
{
  //TODO_COMMENT("We are doing only ROM reading for now");
}

u16 busRead16(u16 ADDRESS)
{
  u16 lo = busRead(ADDRESS);
  u16 hi = busRead(ADDRESS + 1);
  return lo | (hi << 8);
}

void busWrite16(u16 ADDRESS, u16 VALUE)
{
  busWrite(ADDRESS + 1, (VALUE >> 8) & 0xFF);
  busWrite(ADDRESS, VALUE & 0xFF);
}
