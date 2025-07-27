#include "../../include/mappers.h"
#include "../../include/cartridge.h"
#include "../../include/common.h"

u8 mbc1Read(u16 ADDRESS)
{
  CartContext* ctx = cartridgeGetContext();
  // - - - rom bank 00 (fixed)
  if (BETWEEN(ADDRESS, 0x0000, 0x3FFF)) return ctx->romData[ADDRESS];

  // - - - switched rom bank 
  if (BETWEEN(ADDRESS, 0x4000, 0x7FFF)) 
  {
    u32 bankOffset = (u32) ctx->mapperState.mbc1.currentRomBank * 0x4000;
    u32 romAddress = bankOffset + (ADDRESS - 0x4000);

    if (romAddress < ctx->romSize) return ctx->romData[romAddress];
  }

  // - - - external ram 
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF))
  {
    if (ctx->mapperState.mbc1.ramEnabled && ctx->externalRamData)
    {
      u32 ramOffset  = (u32) ctx->mapperState.mbc1.currentRamBank;
      u32 ramAddress = ramOffset + (ADDRESS - 0xA000);
      if (ramAddress < ctx->externalRamSize) return ctx->externalRamData[ramAddress];
    }
    FORGE_LOG_WARNING("MBC1 : Attempted read from idsbaled or out of bounds RAM at 0x%04X", ADDRESS);
  }
  return 0xFF;
}

void mbc1Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = cartridgeGetContext();

  // - - - ram enable 
  if (BETWEEN(ADDRESS, 0x0000, 0x1FFF))
  {
    ctx->mapperState.mbc1.ramEnabled = ((VALUE & 0x0F) == 0x0A);
    return;
  }

  // - - - Rom bank number 
  if (BETWEEN(ADDRESS, 0x4000, 0x5FFF))
  {
    u8 bank = VALUE & 0x1F; 
    if (bank == 0) bank = 1; 

    if (ctx->mapperState.mbc1.romBankingMode)
    {
      ctx->mapperState.mbc1.currentRomBank = (ctx->mapperState.mbc1.currentRamBank << 5) | bank;
    }
    else 
    {
      ctx->mapperState.mbc1.currentRomBank = bank;
    }

    u16 numRomBanks = ctx->romSize / 0x4000;
    ctx->mapperState.mbc1.currentRomBank %= numRomBanks;
    if (ctx->mapperState.mbc1.currentRomBank == 0) ctx->mapperState.mbc1.currentRomBank = 1;
    return;
  }

  // - - - ram bank number 0-3 
  if (BETWEEN(ADDRESS, 0x4000, 0x5FFF))
  {
    if (ctx->mapperState.mbc1.romBankingMode)
    {
      ctx->mapperState.mbc1.currentRomBank = VALUE & 0x03;
      u16 lowerRomBankBits = ctx->mapperState.mbc1.currentRomBank & 0x1F;
      if (lowerRomBankBits == 0) lowerRomBankBits = 1;
      ctx->mapperState.mbc1.currentRomBank = (ctx->mapperState.mbc1.currentRamBank << 5) | lowerRomBankBits;

      u16 numRomBanks = ctx->romSize / 0x4000;
      ctx->mapperState.mbc1.currentRomBank %= numRomBanks;

      if (ctx->mapperState.mbc1.currentRomBank == 0) ctx->mapperState.mbc1.currentRomBank = 1;
    }
    else 
    {
      ctx->mapperState.mbc1.currentRamBank = VALUE & 0x03;
    }
    return;
  }

  // - - - banking mode select 
  if (BETWEEN(ADDRESS, 0x6000, 0x7FFF))
  {
    ctx->mapperState.mbc1.romBankingMode = ((VALUE & 0x01) == 0);
    return;
  }

  // - - - external ram write 
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF))
  {
    // - - - external ram write 
    if (ctx->mapperState.mbc1.ramEnabled && ctx->externalRamData)
    {
      u32 ramOffset  = (u32) ctx->mapperState.mbc1.currentRamBank * 0x2000;
      u32 ramAddress = ramOffset + (ADDRESS - 0xA000);
      if (ramAddress < ctx->externalRamSize)
      {
        ctx->externalRamData[ramAddress] = VALUE;
      }
      else 
      {
        FORGE_LOG_WARNING("MBC1 : Attempted write out of bounds RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      }
    }
    else 
    {
      FORGE_LOG_WARNING("MBC1 : Attempted write to disabled RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE);
    }
    return;
  }

  FORGE_LOG_WARNING("MBC1 : Attempted write to unhandled RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE);
}
