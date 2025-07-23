#include "../include/ppu.h"
#include "../include/lcd.h"
#include <string.h>
#include "../include/ppuStateMachine.h"

// - - - context 
static PPUcontext ctx;

PPUcontext *ppuGetContext() 
{ return &ctx; }


// - - - PPU functions - - - 

// - - - ppu init
void ppuInit() 
{
  ctx.currentFrame        = 0;
  ctx.lineTicks           = 0;
  ctx.frameBuffer         = (u32*) malloc(YRES * XRES * sizeof(u32));

  ctx.pfc.lineX           = 0;
  ctx.pfc.pushedX         = 0;
  ctx.pfc.fetchX          = 0;
  ctx.pfc.pixelFifo.size  = 0;
  ctx.pfc.pixelFifo.head  = ctx.pfc.pixelFifo.tail = NULL;
  ctx.pfc.curFetchState   = FS_TILE;

  ctx.lineSprites         = 0;
  ctx.fetchedEntryCount   = 0;

  lcdInit();
  LCD_STAT_MODE_SET(MODE_OAM);

  memset(ctx.oamRAM,      0, sizeof(ctx.oamRAM));
  memset(ctx.frameBuffer, 0, YRES * XRES * sizeof(u32));
}

// - - - ppu tick
void ppuTick() 
{
  ctx.lineTicks++;

  switch(LCD_STAT_MODE) 
  {
    case MODE_OAM    : { ppuModeOAM();    break; } 
    case MODE_XFER   : { ppuModeXfer();   break; }
    case MODE_VBLANK : { ppuModeVblank(); break; }
    case MODE_HBLANK : { ppuModeHblank(); break; }
  }
}


// - - - OAM I/O - - - 

void ppuOAMwrite(u16 ADDRESS, u8 VALUE) 
{
  if (ADDRESS >= 0xFE00) ADDRESS -= 0xFE00;

  u8* p      = (u8 *)ctx.oamRAM;
  p[ADDRESS] = VALUE;
}

u8 ppuOAMread(u16 ADDRESS) 
{
  if (ADDRESS >= 0xFE00) ADDRESS -= 0xFE00;

  u8* p = (u8 *)ctx.oamRAM;
  return p[ADDRESS];
}


// - - - VRAM I/O - - -

void ppuVRAMwrite(u16 ADDRESS, u8 VALUE) 
{ ctx.vram[ADDRESS - 0x8000] = VALUE; }

u8 ppuVRAMread(u16 ADDRESS) 
{ return ctx.vram[ADDRESS - 0x8000]; }
