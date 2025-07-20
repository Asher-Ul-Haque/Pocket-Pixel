#include "../include/ppu.h"
#include "../include/directMemAccess.h"
#include "../include/lcd.h"
#include "../include/ppuStateMachine.h"
#include <cstring>

static PPUcontext ctx;


PPUcontext* ppuGetContext()
{ return &ctx; }

void ppuInit(u32* FRAME_BUFFER)
{
  ctx.currentFrame  = 0;
  ctx.lineTicks     = 0;
  ctx.frameBuffer   = FRAME_BUFFER;
  
  lcdInit();
  LCD_STAT_MODE_SET(MODE_OAM);
  memset(ctx.oamRam, 0, sizeof(ctx.oamRam));
  memset(ctx.frameBuffer, 0, sizeof(u32) * X_RES * Y_RES);
}

void ppuTick()
{
  ctx.lineTicks++;

  switch(LCD_STAT_MODE)
  {
    case MODE_OAM     : { ppuModeOAM();    break; }
    case MODE_XFER    : { ppuModeXfer();   break; }
    case MODE_VBLANK  : { ppuModeVblank(); break; }
    case MODE_HBLANK  : { ppuModeHblank(); break; }
  }
}

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
