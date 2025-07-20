#include "../include/ppu.h"
#include "../include/directMemAccess.h"

static PPUcontext ctx;

void ppuInit()
{ FORGE_LOG_TRACE("PPU init"); }

void ppuTick()
{ FORGE_LOG_TRACE("PPU tick"); }

void ppuOAMwrite(u16 ADDRESS, u8 VALUE)
{
  if (dmaTransferring())  return;
  if (ADDRESS >= 0xFE00)  ADDRESS -= 0xFE00;

  u8* p = (u8*)ctx.oamRam;
  p[ADDRESS] = VALUE;
}

u8 ppuOAMread(u16 ADDRESS)
{
  if (dmaTransferring())  return 0xFF;
  if (ADDRESS >= 0xFE00)  ADDRESS -= 0xFE00;

  u8* p = (u8*)ctx.oamRam;
  return p[ADDRESS];
}

void ppuVRAMwrite(u16 ADDRESS, u8 VALUE)
{ ctx.vram[ADDRESS - 0x8000] = VALUE; }

u8 ppuVRAMread(u16 ADDRESS)
{ return ctx.vram[ADDRESS - 0x8000]; }
