#include <debug.h>
#include <cartridge/cartridge.h>
#include <ram.h>
#include <bus.h>
#include <timer.h>
#include <cpu/interrupts.h>
#include <ppu/ppu.h>

u16 busRead16(u16 ADDRESS)
{
  // - - - Game Boy is little endian
  u16 lo = busRead(ADDRESS);
  u16 hi = busRead((u16)(ADDRESS + 1));
  return (hi << 8) | lo;
}

void busWrite16(u16 ADDRESS, u16 VALUE)
{
  // - - - Game Boy is little endian
  u8 lo = (u8)(VALUE & 0x00FF);
  u8 hi = (u8)((VALUE & 0xFF00) >> 8);
  busWrite(ADDRESS, lo);
  busWrite((u16)(ADDRESS + 1), hi);
}

u8 busReadRaw(u16 ADDRESS) 
{
  // - - - 1. Serial
  #ifdef DEBUG
    if (ADDRESS == 0xFF01 || ADDRESS == 0xFF02)
    {
      return serialRead(ADDRESS);
    }
  #endif

  // - - - 2. ROM Range (0x0000 - 0x7FFF)
  if (ADDRESS <= BUS_ADDR_ROM_END) 
  { return cartridgeRead(ADDRESS); }

  // - - - 3. VRAM Range (0x8000 - 0x9FFF) - Mode dependent
  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END) 
  { return ppuRead(ADDRESS); }

  // - - - 4. External Cartridge RAM (0xA000 - 0xBFFF)
  if (ADDRESS >= BUS_ADDR_CART_RAM_START && ADDRESS <= BUS_ADDR_CART_RAM_END) 
  { return cartridgeRead(ADDRESS); }

  // - - - 5. Work RAM (0xC000 - 0xDFFF)
  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END) 
  { return ramRead(ADDRESS); }

  // - - - 6. Echo RAM (0xE000 - 0xFDFF)
  if (ADDRESS >= BUS_ADDR_ECHO_START && ADDRESS <= BUS_ADDR_ECHO_END) 
  { return ramRead(ADDRESS - BUS_ADDR_ECHO_OFFSET); }

  // - - - 7. OAM (0xFE00 - 0xFE9F) - Mode dependent
  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END) 
  { return ppuRead(ADDRESS); }

  // - - - 8. Unusable Area (0xFEA0 - 0xFEFF)
  if (ADDRESS >= BUS_ADDR_UNUSED_START && ADDRESS <= BUS_ADDR_UNUSED_END) 
  { return OPEN_BUS_VALUE; }

  // - - - 9. I/O Registers (0xFF00 - 0xFF7F)
  if (ADDRESS >= BUS_ADDR_IO_START && ADDRESS <= BUS_ADDR_IO_END) 
  {
    // - - - Joypad 
    if (ADDRESS == 0xFF00) 
    { return OPEN_BUS_VALUE;
      // TODO_COMMENT("Implement joypad input handling");
    }

    // - - - PPU Io 
    if (ADDRESS >= LCDC && ADDRESS <= WX)
    { return ppuRead(ADDRESS); }

    // - - - Timer Registers (0xFF04 - 0xFF07)
    if (ADDRESS >= DIV_REGISTER_ADDRESS && ADDRESS <= TAC_REGISTER_ADDRESS) 
    { return timerRead(ADDRESS); }

    // - - - Interrupt Flag (0xFF0F)
    if (ADDRESS == ADDR_IF) return cpuReadInterrupt(ADDRESS);

    return OPEN_BUS_VALUE;
  }

  // - - - 10. High RAM (0xFF80 - 0xFFFE)
  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END) 
  { return ramRead(ADDRESS); }

  // - - - 11. Interrupt Enable Register (0xFFFF)
  if (ADDRESS == ADDR_IE) 
  { return cpuReadInterrupt(ADDRESS); }

  return OPEN_BUS_VALUE;
}

u8 busRead(u16 ADDRESS)
{

  return busReadRaw(ADDRESS);
}

void busWrite(u16 ADDRESS, u8 VALUE) 
{
  #ifdef DEBUG
    if (ADDRESS == 0xFF01 || ADDRESS == 0xFF02)
    {
      serialWrite(ADDRESS, VALUE);
      return;
    }
  #endif
  // - - - 1. OAM DMA Lockout

  // - - - 2. ROM Range (Mapper writes)
  if (ADDRESS <= BUS_ADDR_ROM_END) 
  {
    cartridgeWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 3. VRAM Range
  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END) 
  {
    ppuWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 4. External Cartridge RAM
  if (ADDRESS >= BUS_ADDR_CART_RAM_START && ADDRESS <= BUS_ADDR_CART_RAM_END) 
  {
    cartridgeWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 5. Work RAM
  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END) 
  {
    ramWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 6. Echo RAM
  if (ADDRESS >= BUS_ADDR_ECHO_START && ADDRESS <= BUS_ADDR_ECHO_END) 
  {
    ramWrite(ADDRESS - BUS_ADDR_ECHO_OFFSET, VALUE);
    return;
  }

  // - - - 7. OAM 
  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END) 
  {
    ppuWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 8. Unusable Area (Ignored)
  if (ADDRESS >= BUS_ADDR_UNUSED_START && ADDRESS <= BUS_ADDR_UNUSED_END) 
  { return; }

  // - - - 9. I/O Registers
  if (ADDRESS >= BUS_ADDR_IO_START && ADDRESS <= BUS_ADDR_IO_END) 
  {
    if (ADDRESS >= DIV_REGISTER_ADDRESS && ADDRESS <= TAC_REGISTER_ADDRESS) 
    {
      timerWrite(ADDRESS, VALUE);
      return;
    }

    if (ADDRESS == ADDR_IF) 
    {
      cpuWriteInterrupt(ADDRESS, VALUE);
      return;
    }

    if (ADDRESS >= LCDC && ADDRESS <= WX)
    {
      ppuWrite(ADDRESS, VALUE);
      return;
    }

    return;
  }

  // - - - 10. High RAM
  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END) 
  {
    ramWrite(ADDRESS, VALUE);
    return;
  }

  // - - - 11. Interrupt Enable
  if (ADDRESS == ADDR_IE) 
  {
    cpuWriteInterrupt(ADDRESS, VALUE);
    return;
  }
}
