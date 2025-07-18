#include "../include/ram.h"
#include "../../ForgeLibrary/include/asserts.h"

typedef struct 
{
  u8 wram[0x2000];
  u8 hram[0x80];
} RAMcontext;

static RAMcontext ctx;

u8 wramRead(u16 ADDRESS) 
{
  ADDRESS -= 0xC000;

  if (ADDRESS >= 0x2000) 
  {
    FORGE_LOG_FATAL("INVALID WRAM ADDR %08X\n", ADDRESS + 0xC000);
    FORGE_ASSERT(false);
  }

  return ctx.wram[ADDRESS];
}

void wramWrite(u16 ADDRESS, u8 VALUE) 
{
  ADDRESS           -= 0xC000;
  ctx.wram[ADDRESS]  = VALUE;
}

u8 hramRead(u16 ADDRESS) 
{
  ADDRESS -= 0xFF80;
  return ctx.hram[ADDRESS];
}

void hramWrite(u16 ADDRESS, u8 VALUE) 
{
  ADDRESS           -= 0xFF80;
  ctx.hram[ADDRESS]  = VALUE;
}
