#include "../include/bus.h"
#include "../include/cartridge.h"
#include "../include/ram.h"
#include "../include/io.h"
#include "../../../ForgeLibrary/include/asserts.h"

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
  // - - - ROM 
  if (ADDRESS < 0x8000)  return cartridgeRead(ADDRESS);
  // - - - Char / Map data 
  if (ADDRESS < 0xA000)  TODO_COMMENT("Reading Char / Map data");
  // - - - Cartridge Ram 
  if (ADDRESS < 0xC000)  return cartridgeRead(ADDRESS);
  // - - - Working RAM (WRAM)
  if (ADDRESS < 0xE000)  return wramRead(ADDRESS);
  // - - - reserved echo ram
  if (ADDRESS < 0xFE00)  return 0;
  // - - - oam 
  if (ADDRESS < 0xFEA0)  TODO_COMMENT("Reading OAM");
  // - - - reserved unusable 
  if (ADDRESS < 0xFF00)  return 0;
  // - - - IO registers 
  if (ADDRESS < 0xFF80)  return IOread(ADDRESS);
  // - - - last byte : interrupt 
  if (ADDRESS == 0xFFFF) TODO_COMMENT("Reading the master interrupt");

  return hramRead(ADDRESS);
}

void busWrite(u16 ADDRESS, u8 VALUE)
{
  if (ADDRESS < 0x8000)  { cartridgeWrite(ADDRESS, VALUE); return; }
  // - - - Char / Map data 
  if (ADDRESS < 0xA000)  TODO_COMMENT("Writing Char / Map data");
  // - - - Cartridge Ram 
  if (ADDRESS < 0xC000)  { cartridgeWrite(ADDRESS, VALUE); return; }
  // - - - Working RAM (WRAM)
  if (ADDRESS < 0xE000)  { wramWrite(ADDRESS, VALUE);      return; }
  // - - - reserved echo ram
  if (ADDRESS < 0xFE00)  TODO_COMMENT("Writing Echo ram");
  // - - - oam 
  if (ADDRESS < 0xFEA0)  TODO_COMMENT("Writing OAM ram");
  // - - - reserved unusable 
  if (ADDRESS < 0xFF00)  TODO_COMMENT("Writing Unusable Reserved");
  // - - - IO registers 
  if (ADDRESS < 0xFF80)  return IOwrite(ADDRESS, VALUE);
  // - - - last byte : interrupt 
  if (ADDRESS == 0xFFFF) TODO_COMMENT("Writing the master interrupt");

  hramWrite(ADDRESS, VALUE);
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
