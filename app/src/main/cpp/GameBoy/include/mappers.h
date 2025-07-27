#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


extern const u32 ROM_SIZE_MAP[];
extern const u32 RAM_SIZE_MAP[];

// - - - Enum to represent the different MBC types
typedef enum 
{
  MAPPER_NONE, 
  MAPPER_MBC1,
  MAPPER_MBC2,
  MAPPER_MBC3,
  MAPPER_MBC5,
  MAPPER_UNKNOWN 
} MapperType;


// - - - MBC-specific state structs - - - 

// - - - MBC0 (ROM ONLY) has no specific state beyond the base cartridge info.
typedef struct 
{
  u8 dummy; // To make the struct non-empty (empty structs have differnt sizes in c++ and c)
} MBC0State;

// - - - MBC1 State
typedef struct 
{
  bool     ramEnabled;     // - - - Flag for RAM read/write enable
  u16      currentRomBank; // - - - Current active ROM bank (1-based for MBCs)
  u8       currentRamBank; // - - - Current active RAM bank (0-3)
  bool     romBankingMode; // - - - True for ROM banking (16MB ROM), false for RAM banking (256KB ROM + 32KB RAM)
} MBC1State;

// - - - MBC2 State
typedef struct 
{
  bool ramEnabled;     // - - - Flag for RAM read/write enable
  u16  currentRomBank; // - - - Current active ROM bank (1-based, lower 4 bits)
  u8*  internalRam;    // - - - Pointer to 512 nibbles (256 bytes) of internal RAM
} MBC2State;

// - - - MBC3 State
typedef struct 
{
  bool    ramEnabled;     // - - - Flag for RAM read/write enable
  u16     currentRomBank; // - - - Current active ROM bank (1-127)
  u8      currentRamBank; // - - - Current active RAM bank (0-3) or RTC register select (0x08-0x0C)
  
  // - - - Real-Time Clock (RTC) registers
  u8      rtcSeconds;    
  u8      rtcMinutes;   
  u8      rtcHours;    
  u8      rtcDayLow;  
  u8      rtcDayHigh;
  
  // - - - Latched RTC registers - values captured at latching
  u8 latchedRTCseconds;
  u8 latchedRTCminutes;
  u8 latchedRTChours;
  u8 latchedRTCdayLow;
  u8 latchedRTCdayHigh;

  bool    rtcLatched;    
  time_t  lastRTCsystemTime;
} MBC3State;

// - - - MBC5 State
typedef struct 
{
  bool ramEnabled;     // - - - Flag for RAM read/write enable
  u16  currentRomBank; // - - - Current active ROM bank (0-511, 9-bit)
  u8   currentRamBank; // - - - Current active RAM bank (0-15)
} MBC5State;


// - - - Functions - - - 

// - - - Read functions 
FORGE_API u8 mbc0Read(u16 ADDRESS);
FORGE_API u8 mbc1Read(u16 ADDRESS);
FORGE_API u8 mbc2Read(u16 ADDRESS);
FORGE_API u8 mbc3Read(u16 ADDRESS);
FORGE_API u8 mbc5Read(u16 ADDRESS);

// - - - write functions 
FORGE_API void mbc0Write(u16 ADDRESS, u8 VALUE);
FORGE_API void mbc1Write(u16 ADDRESS, u8 VALUE);
FORGE_API void mbc2Write(u16 ADDRESS, u8 VALUE);
FORGE_API void mbc3Write(u16 ADDRESS, u8 VALUE);
FORGE_API void mbc5Write(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
