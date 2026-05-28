/**
 * @file memoryBankController.h
 * @brief Mapper (MBC) types/state for Game Boy Cartridges.
 *
 * This header is intentionally focused on:
 * - Mapper enumeration
 * - Mapper-specific state structs
 * - Small constants related to mapper behavior 
 *
 * The cartridge core owns a mapper state instance and calls into mapper logic
 * @see mapper.h
*/

#pragma once 
#include <common.h>
#include <time.h>

/// @brief Enumeration of supported mappers 
typedef enum 
{
  MAPPER_NONE,              ///< No mapper, simple ROM only cartridge
  MAPPER_MBC1,              ///< MBC1 mapper, supports up to 2MB ROM and 32KB RAM, with bank switching for both ROM and RAM
  MAPPER_MBC1M,             ///< MBC1 with multiple games
  MAPPER_MBC2,              ///< MBC2 mapper, supports up to 512KB ROM and 512 bytes of RAM, with bank switching for ROM only (RAM is fixed)
  MAPPER_MBC3,              ///< MBC3 mapper, supports up to 2MB ROM and 32KB RAM, with bank switching for both ROM and RAM, and also includes a real-time clock (RTC) feature
  MAPPER_MBC5,              ///< MBC5 mapper, supports up to 8MB ROM and 128KB RAM, with bank switching for both ROM and RAM, and also includes support for rumble features in some cartridges
  MAPPER_UNKNOWN 
} MapperType; 


// - - - Mapper States - - - 

/**
 * @brief State structure for MBC0 (no mapper) cartridges, which have no bank switching and a fixed memory layout. 
 * Small games of note more than 32 KiB ROM do not require a MBC chip for ROM
 * banking. The ROM is directly mapped to memory at $0000-$7FFF. Optionally up to 8
 * KiB of RAM could be connected at $A000-BFFF, using a discrete logic decoder in
 * place of a full MBC chip. These cartridges are often referred to as "ROM ONLY" cartridges, 
 * eg: 
*/
typedef struct MBC0State 
{ u8 unused; } MBC0State;


/// @brief Type of banking mode for MBC1 cartridges,
typedef enum MBC1BankMode
{
  MBC1_BANK_MODE_ROM = 0, ///< ROM banking mode: allows switching between different 16KB ROM banks at $4000-$7FFF, while the first 16KB bank is fixed at $0000-$3FFF. RAM banking is disabled in this mode. (16 MB ROM)
  MBC1_BANK_MODE_RAM = 1  ///< RAM banking mode: allows switching between different 8KB RAM banks at $A000-$BFFF, while the ROM banking is limited to the first 128KB (banks 0-7) at $4000-$7FFF. (256 KB ROM + 32 KB RAM) 
} MBC1BankMode;

/// @brief State structure for MBC1 cartridges,
typedef struct MBC1State 
{
  u8            romBankLow5; ///< The lower 5 bits of the ROM bank number, which can be set to select one of the 32 possible ROM banks (0-31). However, due to hardware limitations, certain values (0x00, 0x20, 0x40, 0x60) are not valid and will be remapped to the next valid bank (1, 21, 33, 49 respectively).
  u8            bankHi2;     ///< The upper 2 bits of the ROM bank number, which can be set to select one of the additional ROM banks (banks 32-63) when in ROM banking mode, or to select one of the RAM banks (0-3) when in RAM banking mode.
  MBC1BankMode  bankMode;    ///< The current banking mode of the MBC1 cartridge, which determines how the ROM and RAM banks are switched. In ROM banking mode, the upper 2 bits of the ROM bank number are used to select additional ROM banks, while in RAM banking mode, they are used to select RAM banks.
} MBC1State;

typedef struct MBC2State
{
  u8 romBankLow4; ///< The lowerr 4 bits (bank 0 becomes bank 1)
  u8 ram[512];    ///< MBC2 has built in 512x4 bits of RAM, which is not banked and is accessed at $A000-$A1FF. Only the lower 4 bits of each byte are used for RAM storage, while the upper 4 bits are typically ignored or used for other purposes by the cartridge hardware.
} MBC2State;


// - - - MBC3 - - - 

#define MBC3_RTC_SECONDS_REGISTER 0x08  ///< RTC seconds register, which holds the current seconds value (0-59) of the real-time clock. This register is updated every second when the RTC is running.
#define MBC3_RTC_MINUTES_REGISTER 0x09  ///< RTC minutes register, which holds the current minutes value (0-59) of the real-time clock. This register is updated every minute when the RTC is running.
#define MBC3_RTC_HOURS_REGISTER   0x0A  ///< RTC hours register, which holds the current hours value (0-23) of the real-time clock. This register is updated every hour when the RTC is running.
#define MBC3_RTC_DAY_COUNTER_LOW  0x0B  ///< RTC lower 8 bits of the day counter, which holds the lower 8 bits of the day count (0-255) for the real-time clock. This register is updated every day when the RTC is running.
#define MBC3_RTC_DAY_COUNTER_HI   0x0C  ///< RTC upper 1 bit of the day counter and control flags, which holds the upper 1 bit of the day count (bit 0) and control flags (bits 6-7) for the real-time clock. The control flags include the halt flag (bit 6) and the day carry flag (bit 7), which indicate whether the RTC is halted or has overflowed past 511 days, respectively.

typedef struct MBC3State
{
  u8    romBank7;         ///< 1...127 (0 becomes 1)
  u8    ramBankOrRtcReg;  ///< 0...3 selects RAM bank, 0x08...0x0C selects RTC reg
  u8    latchPrev;        ///< last value written to latch addr (0x6000-0x7FFF)
  bool  latched;          ///< whether latched regs are active

  // - - - RTC live registers
  u8  rtcSeconds;           ///< 0-59
  u8  rtcMinutes;           ///< 0-59
  u8  rtcHours;             ///< 0-23
  u16 rtcDays;              ///< 0-511 (9-bit)

  bool rtcHalt;             ///< halt flag
  bool rtcDayCarry;         ///< day carry flag

  /// - - - RTC latched registers
  u8  latchedSeconds;       
  u8  latchedMinutes; 
  u8  latchedHours;
  u16 latchedDays;

  bool latchedHalt;
  bool latchedDayCarry;

  time_t lastSystemTime;
} MBC3State;

/// @brief State structure for MBC5 cartridges, which support a larger ROM and RAM size compared to previous MBC types, and also include support for rumble features in some cartridges. The ROM banking allows for up to 512 ROM banks (0-511) to be switched at $4000-$7FFF, while the RAM banking allows for up to 16 RAM banks (0-15) to be switched at $A000-$BFFF. The MBC5 also includes additional control registers for enabling rumble features in compatible cartridges.
typedef struct MBC5State
{
  u16 romBank9;             ///< 0...511
  u8  ramBank4;             ///< 0...15
} MBC5State;

/**
 * @brief Read functions for each mapper type, 
 * @param ADDRESS The address to read from
 * @return The byte of data read from the cartridge at the specified address.
*/
u8 mbc0Read(u16 ADDRESS);
u8 mbc1Read(u16 ADDRESS);
u8 mbc2Read(u16 ADDRESS);
u8 mbc3Read(u16 ADDRESS);
u8 mbc5Read(u16 ADDRESS);

/**
 * @brief Write functions for each mapper type, 
 * @param ADDRESS The address to write to
 * @param VALUE The byte of data to write to the cartridge at the specified address.
*/
void mbc0Write(u16 ADDRESS, u8 VALUE);
void mbc1Write(u16 ADDRESS, u8 VALUE);
void mbc2Write(u16 ADDRESS, u8 VALUE);
void mbc3Write(u16 ADDRESS, u8 VALUE);
void mbc5Write(u16 ADDRESS, u8 VALUE);


#define RAM_BANK_SIZE 0x2000u

#define ADDR_ROM0_START 0x0000
#define ADDR_ROM0_END   0x3FFF
#define ADDR_ROMX_START 0x4000 
#define ADDR_ROMX_END   0x7FFF 

#define ADDR_RAM_START  0xA000
#define ADDR_RAM_END    0xBFFF

// - - - MBC1 register windows 
#define ADDR_RAM_ENABLE_END 0x1FFF
#define ADDR_ROM_BANK_END   0x3FFF
#define ADDR_BANK_HI_END    0x5FFF
#define ADDR_MODE_END       0x7FFF

// - - - MBC3 register windows
#define ADDR_RAM_RTC_SEL_END 0x5FFF
#define ADDR_LATCH_END       0x7FFF

// - - - MBC5 register windows
#define ADDR_ROM_LO_END   0x2FFF
#define ADDR_ROM_HI_END   0x3FFF
#define ADDR_RAM_BANK_END 0x5FFF

// - - - Register bit masks 
#define MBC_RAM_ENABLE_MASK   0x0Fu
#define MBC_RAM_ENABLE_VALUE  0x0Au
#define MBC1_ROM_LOW5_MASK    0x1Fu 
#define MBC1_BANK_HI2_MASK    0x03u

// - - - MBC3 values 
#define MBC3_ROM_BANK_MASK        0x7Fu // 1..127, 0 becomes 1
#define MBC3_RTC_DH_DAY_HI_BIT    0x01
#define MBC3_RTC_DH_DAY_HALT_BIT  0x40 
#define MBC3_RTC_DH_DAY_CARRY_BIT 0x80

// - - - MBC5 values
#define MBC5_ROM_LO_MASK   0xFFu // bits 0-7 of ROM bank number
#define MBC5_ROM_HI_MASK   0x01u // bit 8 of ROM bank number
#define MBC5_RAM_BANK_MASK 0x0Fu // RAM bank number (0-15)
                                 
// - - - MBC2 values 
#define MBC2_RAM_SIZE_BYTES 512u
#define MBC2_RAM_ADDR_MASK  0x01FFu // 512 bytes of RAM, mirrored in A000-BFFF with mask 
#define MBC2_RAM_ADDR_MAX   0xA1FFu // valid RAM addresses are A000-A1FF, but many carts mirror this through A000-BFFF with mask                                    
#define MBC2_ROM_LOW4_MASK  0x0Fu   // 4 bits of ROM bank number, 0 becomes 1                                    
#define MBC2_A8_BIT_MASK    0x0100u // bit 8 of address determines whether RAM enable or ROM bank select is being accessed                                    

