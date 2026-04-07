#include <bus.h>
#include <utils/asserts.h>
#include <io/memoryBankController.h>
#include <io/cartridge.h>
#include <common.h>


u8 mbc0Read(u16 ADDRESS)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC0] : Cartridge not initialized");

  // - - - ROM: 0000 - 7FFF (no banking, just return from ROM)
  if (ADDRESS <= ADDR_ROMX_END)  
  {
    if ((u32)ADDRESS < ctx->romSize)
    {  return ctx->romData[ADDRESS]; }

    // - - - Out of bounds read from ROM, return open bus value (0xFF)
    return OPEN_BUS_VALUE;
  }

  // - - - External RAM: A000 - BFFF (if present and enabled)
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || ctx->externalRamSize == 0 || ctx->externalRamData == NULL)
    { return OPEN_BUS_VALUE; }

    u32 offset = (u32) (ADDRESS - ADDR_RAM_START); 
    if (offset >= ctx->externalRamSize)
    { return OPEN_BUS_VALUE; }

    return ctx->externalRamData[offset];
  }

  return OPEN_BUS_VALUE;
}

void mbc0Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC0] : Cartridge not initialized");

  // - - - ROM: 0000 - 7FFF (no banking, writes are ignored)
  if (ADDRESS <= ADDR_ROMX_END)  
  { return; }

  // - - - External RAM: A000 - BFFF (if present and enabled)
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->hasRam || ctx->externalRamSize == 0 || ctx->externalRamData == NULL)
    { return; }

    u32 offset = (u32) (ADDRESS - ADDR_RAM_START); 
    if (offset >= ctx->externalRamSize)
    { return; }

    ctx->externalRamData[offset] = VALUE;
    if (ctx->hasBattery) ctx->ramDirty = true;
    return;
  }
}
