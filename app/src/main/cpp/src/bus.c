#include "io/cartridge.h"
#include <bus.h>

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

u8 busRead(u16 ADDRESS)
{
  // - - - Cartridge ROM (and mapper regs are written via busWrite)
  if (ADDRESS <= BUS_ADDR_ROM_END)
  {
    return cartridgeRead(ADDRESS);
  }

  // - - -  Cartridge external RAM
  if (ADDRESS >= BUS_ADDR_CART_RAM_START && ADDRESS <= BUS_ADDR_CART_RAM_END)
  {
    return cartridgeRead(ADDRESS);
  }

  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END)
  {
    TODO_COMMENT("busRead VRAM (PPU)");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END)
  {
    TODO_COMMENT("busRead WRAM");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS >= BUS_ADDR_ECHO_START && ADDRESS <= BUS_ADDR_ECHO_END)
  {
    TODO_COMMENT("busRead Echo RAM (mirror of WRAM)");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END)
  {
    TODO_COMMENT("busRead OAM (PPU)");
    return OPEN_BUS_VALUE;
  }

  // - - - Unusable 
  if (ADDRESS >= BUS_ADDR_UNUSED_START && ADDRESS <= BUS_ADDR_UNUSED_END)
  {
    FORGE_LOG_WARNING("%s", "Attempted to read from unusable memory area (0xFEA0-0xFEFF). Returning open bus value.");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS >= BUS_ADDR_IO_START && ADDRESS <= BUS_ADDR_IO_END)
  {
    TODO_COMMENT("busRead IO registers");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END)
  {
    TODO_COMMENT("busRead HRAM");
    return OPEN_BUS_VALUE;
  }

  if (ADDRESS == BUS_ADDR_IE)
  {
    TODO_COMMENT("busRead IE (0xFFFF)");
    return OPEN_BUS_VALUE;
  }

  return OPEN_BUS_VALUE;
}

void busWrite(u16 ADDRESS, u8 VALUE)
{
  // - - - Cartridge ROM range is also where mapper control registers live 
  if (ADDRESS <= BUS_ADDR_ROM_END)
  {
    cartridgeWrite(ADDRESS, VALUE);
    return;
  }

  // - - -  Cartridge external RAM 
  if (ADDRESS >= BUS_ADDR_CART_RAM_START && ADDRESS <= BUS_ADDR_CART_RAM_END)
  {
    cartridgeWrite(ADDRESS, VALUE);
    return;
  }

  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END)
  {
    TODO_COMMENT("busWrite VRAM (PPU)");
    (void)VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END)
  {
    TODO_COMMENT("busWrite WRAM");
    (void)VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_ECHO_START && ADDRESS <= BUS_ADDR_ECHO_END)
  {
    TODO_COMMENT("busWrite Echo RAM (mirror of WRAM)");
    (void)VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END)
  {
    TODO_COMMENT("busWrite OAM (PPU)");
    (void)VALUE;
    return;
  }

  // - - -  Unusable 
  if (ADDRESS >= BUS_ADDR_UNUSED_START && ADDRESS <= BUS_ADDR_UNUSED_END)
  {
    FORGE_LOG_WARNING("%s", "Attempted to write to unusable memory area (0xFEA0-0xFEFF). Ignoring.");
    (void)VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_IO_START && ADDRESS <= BUS_ADDR_IO_END)
  {
    TODO_COMMENT("busWrite IO registers");
    (void)VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END)
  {
    TODO_COMMENT("busWrite HRAM");
    (void)VALUE;
    return;
  }

  if (ADDRESS == BUS_ADDR_IE)
  {
    TODO_COMMENT("busWrite IE (0xFFFF)");
    (void)VALUE;
    return;
  }
}
