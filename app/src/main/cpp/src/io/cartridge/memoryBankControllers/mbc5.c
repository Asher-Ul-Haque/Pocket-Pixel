#include <bus.h>
#include <utils/asserts.h>
#include <io/memoryBankController.h>
#include <io/cartridge.h>
#include <common.h>

static inline u32 mbc5GetRomBankCount(const CartContext* CTX)
{
  u32 count = CTX->romSize / ROM_BANK_SIZE;
  return (count == 0) ? 1 : count;
}

static inline u32 mbc5GetRamBankCount(const CartContext* CTX)
{
  u32 count = CTX->externalRamSize / RAM_BANK_SIZE;
  return count; // - - - can be 0 
}

static inline u16 mbc5ComputeRomBank(const CartContext* CTX)
{
  u16 bank = (u16)(CTX->mapper.mbc5.romBank9 & 0x01FFu); // - - - 9-bit

  // - - -  Clamp into available ROM banks
  u32 count = mbc5GetRomBankCount(CTX);
  if (count == 0) return 0;

  bank = (u16)(bank % count);
  return bank;
}

static inline u8 mbc5ComputeRamBank(const CartContext* CTX)
{
  u32 count = mbc5GetRamBankCount(CTX);
  if (count == 0) return 0;

  u8 bank = (u8)(CTX->mapper.mbc5.ramBank4 & MBC5_RAM_BANK_MASK);
  bank    = (u8)(bank % count);
  return bank;
}

static inline void mbc5SyncCtxBanks(CartContext* CTX)
{
  CTX->romBank = mbc5ComputeRomBank(CTX);
  CTX->ramBank = mbc5ComputeRamBank(CTX);
}


// - - - Read / Write - - - 

u8 mbc5Read(u16 ADDRESS)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC5] : Cartridge not initialized");

  // - - - ROM reads
  if (ADDRESS <= ADDR_ROMX_END)
  {
    if (ADDRESS <= ADDR_ROM0_END)
    {
      // - - - Fixed bank 0 
      if ((u32)ADDRESS < ctx->romSize) return ctx->romData[ADDRESS];
      return OPEN_BUS_VALUE;
    }

    // - - -  Banked ROM at 4000-7FFF 
    u16 bank        = mbc5ComputeRomBank(ctx);
    u32 addrInBank  = (u32)(ADDRESS - ADDR_ROMX_START);
    u32 romIndex    = (u32)bank * ROM_BANK_SIZE + addrInBank;

    if (romIndex < ctx->romSize) return ctx->romData[romIndex];
    return OPEN_BUS_VALUE;
  }

  // - - - External RAM reads 
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || !ctx->externalRamData || ctx->externalRamSize == 0) return OPEN_BUS_VALUE;
    if (!ctx->ramEnabled) return OPEN_BUS_VALUE;

    u8 bank = mbc5ComputeRamBank(ctx);
    u32 offset = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)bank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize) return ctx->externalRamData[ramIndex];
    return OPEN_BUS_VALUE;
  }

  return OPEN_BUS_VALUE;
}

void mbc5Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = (CartContext*)cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC5] : Cartridge not initialized");

  // - - - 0000-1FFF: RAM enable
  if (ADDRESS <= ADDR_RAM_ENABLE_END)
  {
    ctx->ramEnabled = ((VALUE & MBC_RAM_ENABLE_MASK) == MBC_RAM_ENABLE_VALUE);
    return;
  }

  // - - - 2000-2FFF: ROM bank low 8 bits
  if (ADDRESS <= ADDR_ROM_LO_END)
  {
    u16 hi = (u16)(ctx->mapper.mbc5.romBank9 & 0x0100u);
    u16 lo = (u16)(VALUE & MBC5_ROM_LO_MASK);
    ctx->mapper.mbc5.romBank9 = (u16)(hi | lo);
    mbc5SyncCtxBanks(ctx);
    return;
  }

  // - - - 3000-3FFF: ROM bank high bit 
  if (ADDRESS <= ADDR_ROM_HI_END)
  {
    u16 lo = (u16)(ctx->mapper.mbc5.romBank9 & 0x00FFu);
    u16 hi = (u16)((VALUE & MBC5_ROM_HI_MASK) << 8);
    ctx->mapper.mbc5.romBank9 = (u16)(hi | lo);
    mbc5SyncCtxBanks(ctx);
    return;
  }

  // - - - 4000-5FFF: RAM bank number (and rumble bit on some carts) 
  if (ADDRESS <= ADDR_RAM_BANK_END)
  {
    // - - - TODO: For rumble carts, bit 3 may be rumble enable.
    ctx->mapper.mbc5.ramBank4 = (u8)(VALUE & MBC5_RAM_BANK_MASK);
    mbc5SyncCtxBanks(ctx);
    return;
  }

  // - - - A000-BFFF: External RAM write
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || !ctx->externalRamData || ctx->externalRamSize == 0) return;
    if (!ctx->ramEnabled) return;

    u8  bank     = mbc5ComputeRamBank(ctx);
    u32 offset   = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)bank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize)
    {
      ctx->externalRamData[ramIndex] = VALUE;
      ctx->ramDirty                  = true;
    }
    return;
  }
}
