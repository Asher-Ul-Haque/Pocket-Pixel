#include <bus.h>
#include <ram.h>

static RamContext ctx;

RamContext* ramGetContext(void)
{ return &ctx; }

void ramWrite(u16 ADDRESS, u8 VALUE)
{
  if (ADDRESS >= BUS_ADDR_WRAM_START && ADDRESS <= BUS_ADDR_WRAM_END)
  {
    ctx.wram[ADDRESS - BUS_ADDR_WRAM_START] = VALUE;
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
    return ctx.wram[ADDRESS - BUS_ADDR_WRAM_START];
  }

  if (ADDRESS >= BUS_ADDR_HRAM_START && ADDRESS <= BUS_ADDR_HRAM_END)
  {
    return ctx.hram[ADDRESS - BUS_ADDR_HRAM_START];
  }

  return OPEN_BUS_VALUE;
}
