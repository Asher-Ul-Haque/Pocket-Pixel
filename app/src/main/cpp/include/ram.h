#pragma once

/**
 * @file ram.h
 * @brief RAM access helpers for the Game Boy
 * Just stores the wram and hram arrays and provides read/write helpers. The bus.c module will route reads/writes to these helpers when the address falls in the appropriate range.
*/

#include <common.h>

#define WRAM_SIZE 0x2000 /// @brief 8 kb Work ram
#define HRAM_SIZE 0x80   /// @brief 127 bytes High ram
                        
typedef struct RamContext
{
  u8 wram[WRAM_SIZE]; ///< Work RAM (0xC000-0xDFFF)
  u8 hram[HRAM_SIZE]; ///< High RAM (0xFF80-0xFFFE)
} RamContext;

void ramWrite(u16 ADDRESS, u8 VALUE);
u8   ramRead(u16 ADDRESS);

RamContext* ramGetContext(void);

