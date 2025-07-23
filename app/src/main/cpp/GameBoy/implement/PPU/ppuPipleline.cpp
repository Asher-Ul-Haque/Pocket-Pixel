#include "../../include/ppu.h"
#include "../../include/lcd.h"
#include "../../include/bus.h"


// - - - FIFO Push Pop - - - 

// - - - push
void pixelFifoPush(u32 VALUE) 
{
  FifoEntry* next = (FifoEntry*) malloc(sizeof(FifoEntry));
  next->next      = NULL;
  next->color     = VALUE;

  PPUcontext* ppuCtx = ppuGetContext();

  if (!ppuCtx->pfc.pixelFifo.head) ppuCtx->pfc.pixelFifo.head       = next; 
  else                             ppuCtx->pfc.pixelFifo.tail->next = next;

  ppuCtx->pfc.pixelFifo.tail = next;
  ppuCtx->pfc.pixelFifo.size++;
}

// - - - pop
u32 pixelFifoPop() 
{
  FORGE_ASSERT_MESSAGE(ppuGetContext()->pfc.pixelFifo.size > 0, "Error in pixel fifo");

  PPUcontext* ppuCtx            = ppuGetContext();
  FifoEntry*  popped            = ppuCtx->pfc.pixelFifo.head;
  ppuCtx->pfc.pixelFifo.head    = popped->next;
  ppuCtx->pfc.pixelFifo.size--;

  u32 val = popped->color;
  free(popped);

  return val;
}


// - - - Window - - - 

bool windowVisible()
{
  LCDcontext* lcdCtx = lcdGetContext();

  return LCD_CNTRL_WIN_ENABLE && 
  lcdCtx->windowX >= 0 && lcdCtx->windowX <= XRES + 6 &&
  lcdCtx->windowY >= 0 && lcdCtx->windowY < YRES;
}

void pipelineLoadWindowTile()
{
  if (!LCD_CNTRL_WIN_ENABLE) return;

  LCDcontext* lcdCtx = lcdGetContext();
  PPUcontext* ppuCtx = ppuGetContext();

  u8 windowY = lcdCtx->windowY;
  u8 windowX = lcdCtx->windowX;
  if (ppuCtx->pfc.fetchX + 7 >= windowX &&
      ppuCtx->pfc.fetchX + 7  < windowX + YRES + 14)
  {
    if (lcdCtx->ly >= windowY && lcdCtx->ly < windowY + XRES)
    {
      u8 windowTileY = ppuCtx->windowLine / 8;
      ppuCtx->pfc.bgFetchData[0] = busRead(LCD_CNTRL_WIN_MAP_AREA + ((ppuCtx->pfc.fetchX + 7 - lcdCtx->windowX) / 8) + (windowTileY * 32));

      if (LCD_CNTRL_BGW_DATA_AREA == 0x8800)
      {
        ppuCtx->pfc.bgFetchData[0] += 128;
      }
    }
  }
}


// - - - Sprites - - - 

u32 fetchSpritePixels(i32 BIT, u32 COLOR, u8 BG_COLOR) 
{
  PPUcontext* ppuCtx = ppuGetContext();
  LCDcontext* lcdCtx = lcdGetContext();

  for (i32 i = 0; i < ppuCtx->fetchedEntryCount; i++) 
  {
    // - - - past pixel point already
    i32 spX = (ppuCtx->fetchedEntries[i].x - 8) + ((lcdCtx->scrollX % 8));
    if (spX + 8 < ppuCtx->pfc.fifoX) continue;

    // - - - offset is out of bounds
    i32 offset = ppuCtx->pfc.fifoX - spX;
    if (offset < 0 || offset > 7) continue;

    BIT = (7 - offset);

    if (ppuCtx->fetchedEntries[i].flagXflip) BIT = offset;

    u8    hi          = !!(ppuCtx->pfc.fetchEntryData[i * 2] & (1 << BIT));
    u8    lo          = !!(ppuCtx->pfc.fetchEntryData[(i * 2) + 1] & (1 << BIT)) << 1;
    bool  bgPriority  = ppuCtx->fetchedEntries[i].flagBGpalette;

    if (!(hi|lo)) continue;

    if (!bgPriority || BG_COLOR == 0) 
    {
      COLOR = (ppuCtx->fetchedEntries[i].flagPaletteNo) ? 
               lcdCtx->sp2Colors[hi|lo] : 
               lcdCtx->sp1Colors[hi|lo];

      if (hi|lo) break;
    }
  }

  return COLOR;
}

void pipeleinLoadSpriteTile() 
{
  OAMlineEntry* le = ppuGetContext()->lineSprites;

  PPUcontext* ppuCtx = ppuGetContext();
  while(le) 
  {
    // - - - need to add entry
    i32 spX = (le->entry.x - 8) + (lcdGetContext()->scrollX % 8);
    if ((spX      >= ppuCtx->pfc.fetchX && spX       < ppuCtx->pfc.fetchX + 8) ||
       ((spX + 8) >= ppuCtx->pfc.fetchX && (spX + 8) < ppuCtx->pfc.fetchX + 8)) 
    { ppuCtx->fetchedEntries[ppuCtx->fetchedEntryCount++] = le->entry; }

    le = le->next;

    // - - - max checking 3 sprites on pixels
    if (!le || ppuCtx->fetchedEntryCount >= 3) break;
  }
}

void pipeleinLoadSpriteData(u8 OFFSET) 
{
  i32 curY         = lcdGetContext()->ly;
  u8  spriteHeight = LCD_CNTRL_OBJ_HEIGHT;

  PPUcontext* ppuCtx = ppuGetContext();

  for (i32 i = 0; i < ppuCtx->fetchedEntryCount; i++) 
  {
    u8 ty = ((curY + 16) - ppuCtx->fetchedEntries[i].y) * 2;

    // - - - flipped upside down
    if (ppuCtx->fetchedEntries[i].flagYflip) ty = ((spriteHeight * 2) - 2) - ty;

    // - - - remove last bit
    u8 tileIndex = ppuCtx->fetchedEntries[i].tile;
    if (spriteHeight == 16) tileIndex &= ~(1); 

    ppuCtx->pfc.fetchEntryData[(i * 2) + OFFSET] = 
      busRead(0x8000 + (tileIndex * 16) + ty + OFFSET);
  }
}


// - - - Pipeline - - - 

bool pipelineFifoAdd() 
{
  PPUcontext* ppuCtx = ppuGetContext();
  LCDcontext* lcdCtx = lcdGetContext();

  // - - - fifo is full!
  if (ppuCtx->pfc.pixelFifo.size > 8) return false;

  i32 x = ppuCtx->pfc.fetchX - (8 - (lcdCtx->scrollX % 8));

  for (i32 i = 0; i < 8; i++) 
  {
    i32 bit   = 7 - i;
    u8  hi    = !!(ppuCtx->pfc.bgFetchData[1] & (1 << bit));
    u8  lo    = !!(ppuCtx->pfc.bgFetchData[2] & (1 << bit)) << 1;
    u32 color = lcdCtx->bgColors[hi | lo];

    if (!LCD_CNTRL_BGW_ENABLE) color = lcdCtx->bgColors[0];
    if (LCD_CNTRL_OBJ_ENABLE)  color = fetchSpritePixels(bit, color, hi | lo);

    if (x >= 0) 
    {
      pixelFifoPush(color);
      ppuCtx->pfc.fifoX++;
    }
  }

  return true;
}


void pipelineFetch() 
{
  PPUcontext* ppuCtx = ppuGetContext();
  LCDcontext* lcdCtx = lcdGetContext();

  switch(ppuCtx->pfc.curFetchState) 
  {
    case FS_TILE: 
      {
        ppuCtx->fetchedEntryCount = 0;

        if (LCD_CNTRL_BGW_ENABLE) 
        {
          ppuCtx->pfc.bgFetchData[0] = 
            busRead(LCD_CNTRL_BG_MAP_AREA    + 
                   (ppuCtx->pfc.mapX   / 8)  + 
                   (((ppuCtx->pfc.mapY / 8)) * 32));
        
          if (LCD_CNTRL_BGW_DATA_AREA == 0x8800) ppuCtx->pfc.bgFetchData[0] += 128;
        }

        pipelineLoadWindowTile();

        if (LCD_CNTRL_OBJ_ENABLE && ppuCtx->lineSprites) pipeleinLoadSpriteTile();

        ppuCtx->pfc.curFetchState = FS_DATA0;
        ppuCtx->pfc.fetchX       += 8;
        break;
      } 

    case FS_DATA0: 
      {
        ppuCtx->pfc.bgFetchData[1] = 
          busRead(LCD_CNTRL_BGW_DATA_AREA           +
                  (ppuCtx->pfc.bgFetchData[0] * 16) + 
                  ppuCtx->pfc.tileY);

        pipeleinLoadSpriteData(0);

        ppuCtx->pfc.curFetchState = FS_DATA1;
        break;
      } 

    case FS_DATA1: 
      {
        ppuCtx->pfc.bgFetchData[2] = 
          busRead(LCD_CNTRL_BGW_DATA_AREA          +
                 (ppuCtx->pfc.bgFetchData[0] * 16) + 
                 ppuCtx->pfc.tileY + 1);

        pipeleinLoadSpriteData(1);

        ppuCtx->pfc.curFetchState = FS_IDLE;
        break;
      }

    case FS_IDLE: 
      {
        ppuCtx->pfc.curFetchState = FS_PUSH;
        break;
      }

    case FS_PUSH: 
      {
        if (pipelineFifoAdd()) ppuCtx->pfc.curFetchState = FS_TILE;
        break;
      } 
  }
}

void pipelinePushPixel() 
{
  PPUcontext* ppuCtx = ppuGetContext();
  LCDcontext* lcdCtx = lcdGetContext();

  if (ppuCtx->pfc.pixelFifo.size > 8) 
  {
    u32 pixelData = pixelFifoPop();

    if (ppuCtx->pfc.lineX >= (lcdCtx->scrollX % 8)) 
    {
      ppuCtx->frameBuffer[ppuCtx->pfc.pushedX + (lcdCtx->ly * XRES)] = pixelData;
      ppuCtx->pfc.pushedX++;
    }

    ppuCtx->pfc.lineX++;
  }
}

void pipelineProcess() 
{
  PPUcontext* ppuCtx = ppuGetContext();
  LCDcontext* lcdCtx = lcdGetContext();

  ppuCtx->pfc.mapY  = (lcdCtx->ly         + lcdCtx->scrollY);
  ppuCtx->pfc.mapX  = (ppuCtx->pfc.fetchX + lcdCtx->scrollX);
  ppuCtx->pfc.tileY = ((lcdCtx->ly        + lcdCtx->scrollY) % 8) * 2;

  if (!(ppuCtx->lineTicks & 1)) pipelineFetch();

  pipelinePushPixel();
}

void pipelineFifoReset() 
{
  while(ppuGetContext()->pfc.pixelFifo.size) pixelFifoPop();
  ppuGetContext()->pfc.pixelFifo.head = 0;
}
