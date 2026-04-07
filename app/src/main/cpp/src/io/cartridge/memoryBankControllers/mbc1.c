#include <bus.h>
#include <utils/asserts.h>
#include <io/memoryBankController.h>
#include <io/cartridge.h>
#include <common.h>

static inline u32 mbc1GetRomBankCount(const CartContext* CTX)
{
  u32 count = CTX->romSize / ROM_BANK_SIZE;
  return (count == 0) ? 1 : count;
}

static inline u32 mbc1GetRamBankCount(const CartContext* CTX)
{
  u32 count = CTX->externalRamSize / RAM_BANK_SIZE;
  return count;
}

// - - - Full 5-bit register value (used for 00 -> 01 translation check)
static inline u8 mbc1Low5Full(const CartContext* CTX)
{
  return (u8) (CTX->mapper.mbc1.romBankLow5 & MBC1_ROM_LOW5_MASK);
}

// - - - Selection low bits differe for MBC1M (bit 4 ignored for selection)
static inline u8 mbc1LowBitsForSelection(const CartContext* CTX)
{
  u8 low5 = mbc1Low5Full(CTX);

  // - - - 4 bit selection 
  if (CTX->mapperType == MAPPER_MBC1M) 
  {
    return (u8)(low5 & 0x0F);
  }

  // - - - 5 bit selection
  else return (u8) (low5 & 0x1F);
}

/**
 * Apply the 00->01 translation rules:
 * IMPORTANT: Docs say the FULL t-bit register is compared for this logic 
*/
static inline u8 mbc1Apply00To01(const CartContext* CTX, u8 LOW_SEL)
{
  if (mbc1Low5Full(CTX) == 0) return 1;
  return LOW_SEL;
}

static inline u16 mbc1ComputeRomXBank(const CartContext* CTX)
{
  u8 hi2 = (u8) (CTX->mapper.mbc1.bankHi2 & MBC1_BANK_HI2_MASK);
  u8 lowSel = mbc1LowBitsForSelection(CTX);
  lowSel = mbc1Apply00To01(CTX, lowSel);
  
  u16 bank;
  // - - - MB1M wiring: hi2 -> bits 4-5, lowSel -> bits 0-3
  if (CTX->mapperType == MAPPER_MBC1M)
  {
    bank = (u16) ((hi2 << 4) | (lowSel & 0x0F));
  }
  
  // - - - Normal MBC1: hi2 -> bits 5-6, lowSel -> bit 0-4
  else 
  {
    bank = (u16)((hi2 << 5) | (lowSel & 0x1F));
  }
  bank %= (u16)mbc1GetRomBankCount(CTX);

  // - - - ROMX must not map bank 0; if it does, bump to 1
  if (bank == 0) bank = 1;
  return bank;
}


static inline u8 mbc1ComputeRamBank(const CartContext* CTX)
{
  u32 ramBanks = mbc1GetRamBankCount(CTX);
  if (ramBanks == 0) return 0;

  if (CTX->mapper.mbc1.bankMode == MBC1_BANK_MODE_RAM)
  {
    u8 bank = (u8)(CTX->mapper.mbc1.bankHi2 & MBC1_BANK_HI2_MASK);
    return (u8)(bank % ramBanks);
  }

  return 0;
}

static inline void mbc1SyncCtxBanks(CartContext* CTX)
{
  CTX->romBank = mbc1ComputeRomXBank(CTX);
  CTX->ramBank = mbc1ComputeRamBank(CTX);
}

static inline u16 mbc1ComputeRom0Bank(const CartContext* CTX)
{
  /* Mode 0: ROM0 fixed to bank 0.
   * Mode 1: ROM0 bank uses high register to access 0x20/0x40/0x60 (MBC1)
   *         or 0x10/0x20/0x30 (MBC1M).
   */
  if (CTX->mapper.mbc1.bankMode != MBC1_BANK_MODE_RAM) return 0;

  u8  hi2 = (u8)(CTX->mapper.mbc1.bankHi2 & MBC1_BANK_HI2_MASK);
  u16 bank;

  if (CTX->mapperType == MAPPER_MBC1M)  bank = (u16)(hi2 << 4);
  else                                  bank = (u16)(hi2 << 5);

  bank %= (u16)mbc1GetRomBankCount(CTX);
  return bank;
}


u8 mbc1Read(u16 ADDRESS)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC1] : Cartridge not initialized");

  // - - - ROM (0000-7FFF) 
  if (ADDRESS <= ADDR_ROMX_END)
  {
    u16 bank;
    u32 addrInBank;

    if (ADDRESS <= ADDR_ROM0_END)
    {
      bank = mbc1ComputeRom0Bank(ctx);
      addrInBank = (u32)ADDRESS;
    }
    else
    {
      bank = mbc1ComputeRomXBank(ctx);
      addrInBank = (u32)(ADDRESS - ADDR_ROMX_START);
    }

    u32 romIndex = (u32)bank * ROM_BANK_SIZE + addrInBank;
    if (romIndex < ctx->romSize) return ctx->romData[romIndex];

    return OPEN_BUS_VALUE;
  }

  // - - - External RAM (A000-BFFF) 
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || ctx->externalRamSize == 0 || ctx->externalRamData == NULL)
      return OPEN_BUS_VALUE;

    if (!ctx->ramEnabled) return OPEN_BUS_VALUE;

    u8  ramBank  = mbc1ComputeRamBank(ctx);
    u32 offset   = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)ramBank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize) return ctx->externalRamData[ramIndex];

    return OPEN_BUS_VALUE;
  }

  return OPEN_BUS_VALUE;
}

void mbc1Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = (CartContext*)cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC1] : Cartridge not initialized");

  // - - - 0000-1FFF: RAM enable
  if (ADDRESS <= ADDR_RAM_ENABLE_END)
  {
    ctx->ramEnabled = ((VALUE & MBC_RAM_ENABLE_MASK) == MBC_RAM_ENABLE_VALUE);
    return;
  }

  // - - - 2000-3FFF: ROM bank low register
  if (ADDRESS <= ADDR_ROM_BANK_END)
  {
    ctx->mapper.mbc1.romBankLow5 = (u8)(VALUE & MBC1_ROM_LOW5_MASK);
    mbc1SyncCtxBanks(ctx);
    return;
  }

  // - - - 4000-5FFF: high bits / RAM bank register
  if (ADDRESS <= ADDR_BANK_HI_END)
  {
    ctx->mapper.mbc1.bankHi2 = (u8)(VALUE & MBC1_BANK_HI2_MASK);
    mbc1SyncCtxBanks(ctx);
    return;
  }

  // - - - 6000-7FFF: mode select
  if (ADDRESS <= ADDR_MODE_END)
  {
    ctx->mapper.mbc1.bankMode = (VALUE & 0x01) ? MBC1_BANK_MODE_RAM : MBC1_BANK_MODE_ROM;
    mbc1SyncCtxBanks(ctx);
    return;
  }

  // - - - A000-BFFF: external RAM write
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || ctx->externalRamSize == 0 || ctx->externalRamData == NULL)
      return;

    if (!ctx->ramEnabled) return;

    u8 ramBank   = mbc1ComputeRamBank(ctx);
    u32 offset   = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)ramBank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize)
    {
      ctx->externalRamData[ramIndex] = VALUE;
      ctx->ramDirty                  = true;
    }
    return;
  }
}
