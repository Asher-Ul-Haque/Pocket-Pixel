#include "../../include/mappers.h"
#include "../../include/cartridge.h"
#include "../../include/common.h"

u8 mbc3Read(u16 ADDRESS) 
{
  CartContext* ctx = cartridgeGetContext();

  // - - - ROM Bank 00 (fixed)
  if (BETWEEN(ADDRESS, 0x0000, 0x3FFF)) return ctx->romData[ADDRESS];
  
  // - - - SWTICHED ROM BANK
  if (BETWEEN(ADDRESS, 0x4000, 0x7FFF)) 
  {
    u32 bankOffset = (u32)ctx->mapperState.mbc3.currentRomBank * 0x4000;
    u32 romAddress = bankOffset + (ADDRESS - 0x4000);
    if (romAddress < ctx->romSize) return ctx->romData[romAddress];
  } 

  // - - - External ram or RTC register 
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF)) 
  {
    if (ctx->mapperState.mbc3.ramEnabled) 
    {
      if (ctx->mapperState.mbc3.currentRamBank <= 0x07) 
      { 
        if (ctx->externalRamData) 
        {
          u32 ramOffset  = (u32)ctx->mapperState.mbc3.currentRamBank * 0x2000; 
          u32 ramAddress = ramOffset + (ADDRESS - 0xA000);
          if (ramAddress < ctx->externalRamSize) return ctx->externalRamData[ramAddress];
        }
        FORGE_LOG_WARNING("MBC3: Attempted read from unallocated or out-of-bounds RAM at 0x%04X", ADDRESS);
      } 
      else if (ctx->mapperState.mbc3.currentRamBank >= 0x08 && ctx->mapperState.mbc3.currentRamBank <= 0x0C) 
      {
        if (ctx->mapperState.mbc3.rtcLatched) 
        {
          switch (ctx->mapperState.mbc3.currentRamBank) 
          {
            case 0x08 : return ctx->mapperState.mbc3.latchedRTCseconds;
            case 0x09 : return ctx->mapperState.mbc3.latchedRTCminutes;
            case 0x0A : return ctx->mapperState.mbc3.latchedRTChours;
            case 0x0B : return ctx->mapperState.mbc3.latchedRTCdayLow;
            case 0x0C : return ctx->mapperState.mbc3.latchedRTCdayHigh;
            default: break; // - - - Should not happen with range check
          }
        } 
        else 
        {
          switch (ctx->mapperState.mbc3.currentRamBank) 
          {
            case 0x08 : return ctx->mapperState.mbc3.rtcSeconds;
            case 0x09 : return ctx->mapperState.mbc3.rtcMinutes;
            case 0x0A : return ctx->mapperState.mbc3.rtcHours;
            case 0x0B : return ctx->mapperState.mbc3.rtcDayLow;
            case 0x0C : return ctx->mapperState.mbc3.rtcDayHigh;
            default: break; // - - - Should not happen with range check
          }
        }
      }
    }
    FORGE_LOG_WARNING("MBC3: Attempted read from disabled RAM/RTC or invalid bank at 0x%04X", ADDRESS);
  }
  return 0xFF; 
}

void mbc3Write(u16 ADDRESS, u8 VALUE) 
{
  CartContext* ctx = cartridgeGetContext();

  // - - - RAM and RTC enable
  if (BETWEEN(ADDRESS, 0x0000, 0x1FFF))
  {
    ctx->mapperState.mbc3.ramEnabled = ((VALUE & 0x0F) == 0x0A);

    // - - - ram disable 
    if ((VALUE & 0x0F) == 0x00)
    {
      if (ctx->mapperState.mbc3.ramEnabled && ctx->hasBattery && ctx->ramDirty)
      {
        FORGE_LOG_TRACE("Saving Game : MBC3");
        cartridgeFlushRAM();
      }
      ctx->mapperState.mbc3.ramEnabled = false;
    }
    return;
  }

  // - - - ROM Bank Number (1-127)
  if (BETWEEN(ADDRESS, 0x2000, 0x3FFF))
  {
    ctx->mapperState.mbc3.currentRomBank = VALUE & 0x7F; 
    if (ctx->mapperState.mbc3.currentRomBank == 0) ctx->mapperState.mbc3.currentRomBank = 1; 
    
    u16 numRomBanks = ctx->romSize / 0x4000;
    ctx->mapperState.mbc3.currentRomBank %= numRomBanks; 
    if (ctx->mapperState.mbc3.currentRomBank == 0) ctx->mapperState.mbc3.currentRomBank = 1; 

    return;
  } 

  // - - - RAM Bank Number (0-3) or RTC Register Select (0x08-0x0C)
  if (BETWEEN(ADDRESS, 0x4000, 0x5FFF)) 
  {
    ctx->mapperState.mbc3.currentRamBank = VALUE;
    return;
  }

  // - - - Latch Clock Data (Write 0x00 then 0x01 to latch)
  if (BETWEEN(ADDRESS, 0x6000, 0x7FFF))
  {
    if (VALUE == 0x00) 
    {
      ctx->mapperState.mbc3.rtcLatched = false;
    } 
    else if (VALUE == 0x01 && !ctx->mapperState.mbc3.rtcLatched) 
    {
      // - - - Latch current RTC registers into latchedRTC variables
      ctx->mapperState.mbc3.latchedRTCseconds   = ctx->mapperState.mbc3.rtcSeconds;
      ctx->mapperState.mbc3.latchedRTCminutes   = ctx->mapperState.mbc3.rtcMinutes;
      ctx->mapperState.mbc3.latchedRTChours     = ctx->mapperState.mbc3.rtcHours;
      ctx->mapperState.mbc3.latchedRTCdayLow    = ctx->mapperState.mbc3.rtcDayLow;
      ctx->mapperState.mbc3.latchedRTCdayHigh   = ctx->mapperState.mbc3.rtcDayHigh;
      ctx->mapperState.mbc3.rtcLatched          = true;
    }
    return;
  } 

  // - - - External RAM or RTC Register write
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF)) 
  {
    if (ctx->mapperState.mbc3.ramEnabled) 
    {
      if (ctx->mapperState.mbc3.currentRamBank <= 0x07) 
      { 
        if (ctx->externalRamData) 
        {
          u32 ramOffset     = (u32)ctx->mapperState.mbc3.currentRamBank * 0x2000; // 8KB RAM banks
          u32 ramAddress    = ramOffset + (ADDRESS - 0xA000);

          if (ramAddress < ctx->externalRamSize) 
          { 
            ctx->externalRamData[ramAddress]  = VALUE; 
            ctx->ramDirty                     = true;
          }
          else 
          { FORGE_LOG_WARNING("MBC3: Attempted write out of bounds RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE); }
        }
      } 

      else if (ctx->mapperState.mbc3.currentRamBank >= 0x08 && ctx->mapperState.mbc3.currentRamBank <= 0x0C) 
      {
        switch (ctx->mapperState.mbc3.currentRamBank) 
        {
          case 0x08 : { ctx->mapperState.mbc3.rtcSeconds  = VALUE; break; }
          case 0x09 : { ctx->mapperState.mbc3.rtcMinutes  = VALUE; break; }
          case 0x0A : { ctx->mapperState.mbc3.rtcHours    = VALUE; break; }
          case 0x0B : { ctx->mapperState.mbc3.rtcDayLow   = VALUE; break; }
          case 0x0C : { ctx->mapperState.mbc3.rtcDayHigh  = VALUE; break; }
        }
        ctx->ramDirty = true;
      }
    } 
    else 
    { FORGE_LOG_WARNING("MBC3: Attempted write to disabled RAM/RTC at 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
  } 
  else 
  {  FORGE_LOG_WARNING("MBC3: Attempted write to unhandled address 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
}
