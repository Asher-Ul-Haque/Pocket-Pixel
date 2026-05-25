#pragma once

/**
 * @file ram.h
 * @brief RAM access helpers for the Game Boy
 * Just stores the wram and hram arrays and provides read/write helpers. The bus.c module will route reads/writes to these helpers when the address falls in the appropriate range.
*/

#include <common.h>

#define WRAM_BANK_SIZE  0x1000 /// @brief 4 kb Work RAM bank size
#define WRAM_BANK_COUNT 8      /// @brief CGB has 8 banks, bank 0 fixed at 0xC000
#define WRAM_SIZE       (WRAM_BANK_SIZE * WRAM_BANK_COUNT)
#define HRAM_SIZE       0x80   /// @brief 127 bytes High ram

#define REG_SVBK 0xFF70u ///< CGB WRAM bank register
                        
typedef struct RamContext
{
  u8 wramBanks[WRAM_BANK_COUNT][WRAM_BANK_SIZE]; ///< CGB Work RAM banks
  u8 wramBank;                                    ///< Active switchable bank (1-7)
  u8 hram[HRAM_SIZE]; ///< High RAM (0xFF80-0xFFFE)
} RamContext;

void ramWrite(u16 ADDRESS, u8 VALUE);
u8   ramRead(u16 ADDRESS);

RamContext* ramGetContext(void);

u8   ramReadWramBank(void);
void ramWriteWramBank(u8 VALUE);
