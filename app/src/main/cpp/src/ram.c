#include <bus.h>
#include <ram.h>

static RamContext ctx;

RamContext* ramGetContext(void)
{ return &ctx; }

static u8 ramGetActiveBank(void)
{
  u8 bank = ctx.wramBank & 0x07u;
  return (bank == 0) ? 1 : bank;
}

void ramWrite(u16 ADDRESS, u8 VALUE)
{
  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END)
  {
    u16 offset = (u16)(ADDRESS - BUS_ADDR_WRAM_START);
    if (offset < WRAM_BANK_SIZE)
    {
      ctx.wramBanks[0][offset] = VALUE;
    }
    else
    {
      u16 bankOffset = (u16)(offset - WRAM_BANK_SIZE);
      ctx.wramBanks[ramGetActiveBank()][bankOffset] = VALUE;
    }
    return;
  }

  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END)
  {
    ctx.hram[ADDRESS - BUS_ADDR_HRAM_START] = VALUE;
    return;
  }
}

u8 ramRead(u16 ADDRESS)
{
  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END)
  {
    u16 offset = (u16)(ADDRESS - BUS_ADDR_WRAM_START);
    if (offset < WRAM_BANK_SIZE)
    {
      return ctx.wramBanks[0][offset];
    }

    u16 bankOffset = (u16)(offset - WRAM_BANK_SIZE);
    return ctx.wramBanks[ramGetActiveBank()][bankOffset];
  }

  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END)
  {
    return ctx.hram[ADDRESS - BUS_ADDR_HRAM_START];
  }

  return OPEN_BUS_VALUE;
}

u8 ramReadWramBank(void)
{
  u8 bank = ramGetActiveBank();
  return (u8)(0xF8u | (bank & 0x07u));
}

void ramWriteWramBank(u8 VALUE)
{
  u8 bank = (u8)(VALUE & 0x07u);
  ctx.wramBank = (bank == 0) ? 1 : bank;
}
