#include "../../include/mappers.h"
#include "../../include/cartridge.h"
#include "../../include/common.h"

u8 mbc2Read(u16 ADDRESS) 
{
  CartContext* ctx = cartridgeGetContext();
  
  // - - - ROM Bank 00 (fixed)
  if (BETWEEN(ADDRESS, 0x0000, 0x3FFF))   return ctx->romData[ADDRESS]; 

  // - - - switched rom bank 
  if (BETWEEN(ADDRESS, 0x4000, 0x7FFF)) 
  {
    u32 bankOffset = (u32)ctx->mapperState.mbc2.currentRomBank * 0x4000;
    u32 romAddress = bankOffset + (ADDRESS - 0x4000);

    if (romAddress < ctx->romSize) return ctx->romData[romAddress];
  } 

  // - - - Built-in RAM (512 nibbles, addressable as 256 bytes)
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF))
  {
    if (ctx->mapperState.mbc2.ramEnabled && ctx->mapperState.mbc2.internalRam) 
    {
      u16 ramAddress = (ADDRESS - 0xA000) & 0x1FF; 

      if (ramAddress < 256) return ctx->mapperState.mbc2.internalRam[ramAddress] & 0x0F;
    }
    FORGE_LOG_WARNING("MBC2: Attempted read from disabled or out-of-bounds RAM at 0x%04X", ADDRESS);
  }
  return 0xFF;
}

void mbc2Write(u16 ADDRESS, u8 VALUE) 
{
  CartContext* ctx = cartridgeGetContext();
  
  // - - - RAM Enable / ROM Bank Number (lower 4 bits)
  if (BETWEEN(ADDRESS, 0x0000, 0x3FFF))
  {
    // - - - RAM ENABLE 
    if ((ADDRESS & 0x0100) == 0) 
    {  ctx->mapperState.mbc2.ramEnabled = ((VALUE & 0x0F) == 0x0A); } 

    // - - - ram disable 
    else if ((VALUE & 0x0F) == 0x00)
    {
      if (ctx->mapperState.mbc2.ramEnabled && ctx->hasBattery && ctx->ramDirty)
      {
        FORGE_LOG_TRACE("Saving Game : MBC2")
        cartridgeFlushRAM();
      }
      ctx->mapperState.mbc2.ramEnabled = false;
    }

    // - - - rom bank number
    else 
    { 
      ctx->mapperState.mbc2.currentRomBank = VALUE & 0x0F; 
      if (ctx->mapperState.mbc2.currentRomBank == 0) ctx->mapperState.mbc2.currentRomBank = 1; 
      
      u16 numRomBanks = ctx->romSize / 0x4000;
      ctx->mapperState.mbc2.currentRomBank %= numRomBanks; 
      if (ctx->mapperState.mbc2.currentRomBank == 0) ctx->mapperState.mbc2.currentRomBank = 1; // Ensure it's never 0 if not ROM_ONLY
    }

    return;
  } 

  // - - - buildt in ram write 
  if (BETWEEN(ADDRESS, 0xA000, 0xBFFF)) 
  {
    if (ctx->mapperState.mbc2.ramEnabled && ctx->mapperState.mbc2.internalRam) 
    {
      u16 ramAddress = (ADDRESS - 0xA000) & 0x1FF; 
      if (ramAddress < 256) 
      { 
        ctx->mapperState.mbc2.internalRam[ramAddress] = VALUE & 0x0F; 
        ctx->ramDirty = true;
      } 
      else 
      { FORGE_LOG_WARNING("MBC2: Attempted write out of bounds RAM address 0x%04X with value 0x%02X", ADDRESS, VALUE); }
    } 
    else 
    { FORGE_LOG_WARNING("MBC2: Attempted write to disabled RAM at 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
  } 
  else 
  { FORGE_LOG_WARNING("MBC2: Attempted write to unhandled address 0x%04X with value 0x%02X. Ignoring.", ADDRESS, VALUE); }
}
