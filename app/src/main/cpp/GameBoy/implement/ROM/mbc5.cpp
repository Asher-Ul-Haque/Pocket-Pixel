#include "../../include/mappers.h"
#include "../../include/cartridge.h"
#include "../../include/common.h"


u8 mbc5Read(u16 ADDRESS) 
{
  CartContext* ctx = cartridgeGetContext();

  // - - - ROM Bank 00 (fixed)
  if (BETWEEN(ADDRESS, 0x0000, 0x3FFF))  return ctx->romData[ADDRESS]; 

  // - - - Switched ROM Bank
  if (BETWEEN(ADDRESS, 0x4000, 0x7FFF)) 
  {
    u32 bankOffset = (u32)ctx->mapperState.mbc5.currentRomBank * 0x4000;
    u32 romAddress = bankOffset + (ADDRESS - 0x4000);
    if (romAddress < ctx->romSize)  return ctx->romData[romAddress];
  } 

  // - - - EXTERNA  ram
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF))
  {
    if (ctx->mapperState.mbc5.ramEnabled && ctx->externalRamData) 
    {
      u32 ramOffset  = (u32)ctx->mapperState.mbc5.currentRamBank * 0x2000;
      u32 ramAddress = ramOffset + (ADDRESS - 0xA000);

      if (ramAddress < ctx->externalRamSize) return ctx->externalRamData[ramAddress];
    }
    FORGE_LOG_WARNING("MBC5: Attempted read from disabled or out-of-bounds RAM at 0x%04X", ADDRESS);
  }
  return 0xFF; 
}

void mbc5Write(u16 ADDRESS, u8 VALUE) 
{
  CartContext* ctx = cartridgeGetContext();
      
  // - - - RAM Enable (Write any value with 0x0A in the lower 4 bits to enable RAM)
  if (BETWEEN(ADDRESS, 0x0000, 0x1FFF))
  {
    ctx->mapperState.mbc5.ramEnabled = ((VALUE & 0x0F) == 0x0A);
    return;
  } 

  // - --  ROM bank number
  if (BETWEEN(ADDRESS, 0x2000, 0x2FFF))
  {
    ctx->mapperState.mbc5.currentRomBank = (ctx->mapperState.mbc5.currentRomBank & 0x100) | VALUE; 
    
    u16 numRomBanks = ctx->romSize / 0x4000;
    ctx->mapperState.mbc5.currentRomBank %= numRomBanks; 
    return;
  } 

  // - --  ROM BANK NUMBER 9TH bit
  if (BETWEEN(ADDRESS, 0x3000, 0x3FFF)) 
  {
    ctx->mapperState.mbc5.currentRomBank = (ctx->mapperState.mbc5.currentRomBank & 0xFF) | ((VALUE & 0x01) << 8); 
      
    u16 numRomBanks = ctx->romSize / 0x4000;
    ctx->mapperState.mbc5.currentRomBank %= numRomBanks;  
    return;
  } 

  // - - - ram bank number 
  if (BETWEEN(ADDRESS, 0x4000, 0x5FFF))
  {
    ctx->mapperState.mbc5.currentRamBank = VALUE & 0x0F;
    return;
  } 

  // - - - External RAM write
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF)) 
  {
    if (ctx->mapperState.mbc5.ramEnabled && ctx->externalRamData) 
    {
      u32 ramOffset     = (u32)ctx->mapperState.mbc5.currentRamBank * 0x2000; 
      u32 ramAddress    = ramOffset + (ADDRESS - 0xA000);
      if (ramAddress < ctx->externalRamSize) 
      { ctx->externalRamData[ramAddress] = VALUE; } 
      else 
      { FORGE_LOG_WARNING("MBC5: Attempted write out of bounds RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE); }
    } 
    else 
    { FORGE_LOG_WARNING("MBC5: Attempted write to disabled RAM at 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
    return;
  } 
  else { FORGE_LOG_WARNING("MBC5: Attempted write to unhandled address 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
}
