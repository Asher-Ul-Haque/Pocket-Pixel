#pragma once 
#include <common.h>

#define OPEN_BUS_VALUE 0xFF

#define BUS_ADDR_ROM_END        0x7FFF
#define BUS_ADDR_VRAM_START     0x8000
#define BUS_ADDR_VRAM_END       0x9FFF

#define BUS_ADDR_CART_RAM_START 0xA000
#define BUS_ADDR_CART_RAM_END   0xBFFF

#define BUS_ADDR_WRAM_START     0xC000
#define BUS_ADDR_WRAM_END       0xDFFF

#define BUS_ADDR_ECHO_START     0xE000
#define BUS_ADDR_ECHO_END       0xFDFF
#define BUS_ADDR_ECHO_OFFSET    0x2000

#define BUS_ADDR_OAM_START      0xFE00
#define BUS_ADDR_OAM_END        0xFE9F

#define BUS_ADDR_UNUSED_START   0xFEA0
#define BUS_ADDR_UNUSED_END     0xFEFF

#define BUS_ADDR_IO_START       0xFF00
#define BUS_ADDR_IO_END         0xFF7F

#define BUS_ADDR_HRAM_START     0xFF80
#define BUS_ADDR_HRAM_END       0xFFFE

#define BUS_ADDR_IE             0xFFFF

/**
 * @brief Reads a byte from the bus at the specified address.
 * @param ADDRESS The 16-bit address to read from.
 * @return The byte read from the bus at the specified address. If the address is not
*/
u8 busRead(u16 ADDRESS);

/**
 * @brief Writes a byte to the bus at the specified address.
 * @param ADDRESS The 16-bit address to write to.
 * @param VALUE The byte value to write to the bus at the specified address. If the
*/
void busWrite(u16 ADDRESS, u8 VALUE);

/**
 * @brief Reads a 16-bit value from the bus at the specified address.
 * @param ADDRESS The 16-bit address to read from.
 * @return The 16-bit value read from the bus at the specified address. This is done by reading two consecutive bytes and combining them into a 16-bit value.
 */
u16 busRead16(u16 ADDRESS);

/**
 * @brief Writes a 16-bit value to the bus at the specified address.
 * @param ADDRESS The 16-bit address to write to.
 * @param VALUE The 16-bit value to write to the bus at the specified address.
*/
void busWrite16(u16 ADDRESS, u16 VALUE);
