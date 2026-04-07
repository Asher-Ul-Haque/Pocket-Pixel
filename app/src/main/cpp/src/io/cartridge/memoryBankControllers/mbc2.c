#include <bus.h>
#include <utils/asserts.h>
#include <io/memoryBankController.h>
#include <io/cartridge.h>
#include <common.h>

static inline u32 mbc2GetRomBankCount(const CartContext* CTX)
{
  u32 count = CTX->romSize / ROM_BANK_SIZE;
  return (count == 0) ? 1 : count;
}

static inline u8 mbc2NormalizeRomBank(u8 LOW_4, const CartContext* CTX)
{
  u8 bank = (u8)(LOW_4 & MBC2_ROM_LOW4_MASK);
  if (bank == 0) bank = 1;

  // - --  Clamp to available banks (some ROMs smaller than 16 banks) 
  u32 count = mbc2GetRomBankCount(CTX);
  bank      = (u8)(bank % count);
  if (bank == 0) bank = 1;

  return bank;
}


// - - - Read / Write - - -

u8 mbc2Read(u16 ADDRESS)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC2] : Cartridge not initialized");

  // - - - ROM reads 
  if (ADDRESS <= ADDR_ROMX_END)
  {
    if (ADDRESS <= 0x3FFF)
    {
      // - - - Fixed bank 0
      if ((u32)ADDRESS < ctx->romSize) return ctx->romData[ADDRESS];
      return 0xFF;
    }

    // - - - Banked region 
    u8 bank         = mbc2NormalizeRomBank(ctx->mapper.mbc2.romBankLow4, ctx);
    u32 addrInBank  = (u32)(ADDRESS - ADDR_ROMX_START);
    u32 romIndex    = (u32)bank * ROM_BANK_SIZE + addrInBank;

    if (romIndex < ctx->romSize) return ctx->romData[romIndex];
    return 0xFF;
  }

  // - - - Internal RAM reads (A000-A1FF; many implementations mirror through A000-BFFF with mask)
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->ramEnabled) return 0xFF;

    u16 idx    = (u16)((ADDRESS - ADDR_RAM_START) & MBC2_RAM_ADDR_MASK);
    u8  nibble = (u8)(ctx->mapper.mbc2.ram[idx] & 0x0F);

    // - - - Typical behavior: upper nibble reads as 1s
    return (u8)(0xF0 | nibble);
  }

  return 0xFF;
}

void mbc2Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = (CartContext*)cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC2] : Cartridge not initialized");

  // - - -  0000-3FFF: control registers, but split by A8 (bit 8 of address) 
  if (ADDRESS <= ADDR_ROM_BANK_END)
  {
    // - - - 0000-1FFF: RAM enable when A8=0 
    if (ADDRESS <= ADDR_RAM_ENABLE_END)
    {
      if ((ADDRESS & MBC2_A8_BIT_MASK) == 0)
      {
        ctx->ramEnabled = ((VALUE & MBC_RAM_ENABLE_MASK) == MBC_RAM_ENABLE_VALUE);
      }
      return;
    }

    // - - - 2000-3FFF: ROM bank select when A8=1
    if ((ADDRESS & MBC2_A8_BIT_MASK) != 0)
    {
      ctx->mapper.mbc2.romBankLow4  = (u8)(VALUE & MBC2_ROM_LOW4_MASK);
      ctx->romBank                  = mbc2NormalizeRomBank(ctx->mapper.mbc2.romBankLow4, ctx);
    }

    return;
  }

  // - - - A000-BFFF: internal RAM write (only low nibble stored)
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->ramEnabled) return;

    u16 idx = (u16)((ADDRESS - ADDR_RAM_START) & MBC2_RAM_ADDR_MASK);
    ctx->mapper.mbc2.ram[idx]   = (u8)(VALUE & 0x0F);
    ctx->ramDirty               = true;
    return;
  }
}
