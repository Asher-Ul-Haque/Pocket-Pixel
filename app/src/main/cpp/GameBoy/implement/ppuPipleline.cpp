#include "../include/ppu.h"
#include "../include/lcd.h"
#include "../include/bus.h"


// - - - FIFO Push Pop - - - 

// - - - push
void pixelFifoPush(u32 VALUE) 
{
  FifoEntry* next = (FifoEntry*) malloc(sizeof(FifoEntry));
  next->next      = NULL;
  next->color     = VALUE;

  if (!ppuGetContext()->pfc.pixelFifo.head) ppuGetContext()->pfc.pixelFifo.head = next; 
  else                                      ppuGetContext()->pfc.pixelFifo.tail->next = next;

  ppuGetContext()->pfc.pixelFifo.tail = next;
  ppuGetContext()->pfc.pixelFifo.size++;
}

// - - - pop
u32 pixelFifoPop() 
{
  FORGE_ASSERT_MESSAGE(ppuGetContext()->pfc.pixelFifo.size > 0, "Error in pixel fifo");

  FifoEntry* popped                       = ppuGetContext()->pfc.pixelFifo.head;
  ppuGetContext()->pfc.pixelFifo.head     = popped->next;
  ppuGetContext()->pfc.pixelFifo.size--;

  u32 val = popped->color;
  free(popped);

  return val;
}


// - - - Window - - - 

bool windowVisible()
{
  return LCD_CNTRL_WIN_ENABLE && 
  lcdGetContext()->windowX >= 0 && lcdGetContext()->windowX <= XRES + 6 &&
  lcdGetContext()->windowY >= 0 && lcdGetContext()->windowY < YRES;
}

void pipelineLoadWindowTile()
{
  if (!LCD_CNTRL_WIN_ENABLE) return;

  u8 windowY = lcdGetContext()->windowY;
  u8 windowX = lcdGetContext()->windowX;
  if (ppuGetContext()->pfc.fetchX + 7 >= windowX &&
      ppuGetContext()->pfc.fetchX + 7  < windowX + YRES + 14)
  {
    if (lcdGetContext()->ly >= windowY && lcdGetContext()->ly < windowY + XRES)
    {
      u8 windowTileY = ppuGetContext()->windowLine / 8;
      ppuGetContext()->pfc.bgFetchData[0] = busRead(LCD_CNTRL_WIN_MAP_AREA + ((ppuGetContext()->pfc.fetchX + 7 - lcdGetContext()->windowX) / 8) + (windowTileY * 32));

      if (LCD_CNTRL_BGW_DATA_AREA == 0x8800)
      {
        ppuGetContext()->pfc.bgFetchData[0] += 128;
      }
    }
  }
}


// - - - Sprites - - - 

u32 fetchSpritePixels(i32 BIT, u32 COLOR, u8 BG_COLOR) 
{
  for (i32 i = 0; i < ppuGetContext()->fetchedEntryCount; i++) 
  {
    // - - - past pixel point already
    i32 spX = (ppuGetContext()->fetchedEntries[i].x - 8) + 
              ((lcdGetContext()->scrollX % 8));
    if (spX + 8 < ppuGetContext()->pfc.fifoX) continue;

    // - - - offset is out of bounds
    i32 offset = ppuGetContext()->pfc.fifoX - spX;
    if (offset < 0 || offset > 7) continue;

    BIT = (7 - offset);

    if (ppuGetContext()->fetchedEntries[i].flagXflip) BIT = offset;

    u8    hi          = !!(ppuGetContext()->pfc.fetchEntryData[i * 2] & (1 << BIT));
    u8    lo          = !!(ppuGetContext()->pfc.fetchEntryData[(i * 2) + 1] & (1 << BIT)) << 1;

    bool  bgPriority  = ppuGetContext()->fetchedEntries[i].flagBGpalette;

    if (!(hi|lo)) continue;

    if (!bgPriority || BG_COLOR == 0) 
    {
      COLOR = (ppuGetContext()->fetchedEntries[i].flagPaletteNo) ? 
               lcdGetContext()->sp2Colors[hi|lo] : 
               lcdGetContext()->sp1Colors[hi|lo];

      if (hi|lo) break;
    }
  }

  return COLOR;
}

void pipeleinLoadSpriteTile() 
{
  OAMlineEntry* le = ppuGetContext()->lineSprites;

  while(le) 
  {
    // - - - need to add entry
    i32 spX = (le->entry.x - 8) + (lcdGetContext()->scrollX % 8);
    if ((spX      >= ppuGetContext()->pfc.fetchX && spX       < ppuGetContext()->pfc.fetchX + 8) ||
       ((spX + 8) >= ppuGetContext()->pfc.fetchX && (spX + 8) < ppuGetContext()->pfc.fetchX + 8)) 
    { ppuGetContext()->fetchedEntries[ppuGetContext()->fetchedEntryCount++] = le->entry; }

    le = le->next;

    // - - - max checking 3 sprites on pixels
    if (!le || ppuGetContext()->fetchedEntryCount >= 3) break;
  }
}

void pipeleinLoadSpriteData(u8 OFFSET) 
{
  i32 curY         = lcdGetContext()->ly;
  u8  spriteHeight = LCD_CNTRL_OBJ_HEIGHT;

  for (i32 i = 0; i < ppuGetContext()->fetchedEntryCount; i++) 
  {
    u8 ty = ((curY + 16) - ppuGetContext()->fetchedEntries[i].y) * 2;

    // - - - flipped upside down
    if (ppuGetContext()->fetchedEntries[i].flagYflip) ty = ((spriteHeight * 2) - 2) - ty;

    // - - - remove last bit
    u8 tileIndex = ppuGetContext()->fetchedEntries[i].tile;
    if (spriteHeight == 16) tileIndex &= ~(1); 

    ppuGetContext()->pfc.fetchEntryData[(i * 2) + OFFSET] = 
      busRead(0x8000 + (tileIndex * 16) + ty + OFFSET);
  }
}


// - - - Pipeline - - - 

bool pipelineFifoAdd() 
{
  // - - - fifo is full!
  if (ppuGetContext()->pfc.pixelFifo.size > 8) return false;

  i32 x = ppuGetContext()->pfc.fetchX - (8 - (lcdGetContext()->scrollX % 8));

  for (i32 i = 0; i < 8; i++) 
  {
    i32 bit   = 7 - i;
    u8  hi    = !!(ppuGetContext()->pfc.bgFetchData[1] & (1 << bit));
    u8  lo    = !!(ppuGetContext()->pfc.bgFetchData[2] & (1 << bit)) << 1;
    u32 color = lcdGetContext()->bgColors[hi | lo];

    if (!LCD_CNTRL_BGW_ENABLE) color = lcdGetContext()->bgColors[0];

    if (LCD_CNTRL_OBJ_ENABLE)  color = fetchSpritePixels(bit, color, hi | lo);

    if (x >= 0) 
    {
      pixelFifoPush(color);
      ppuGetContext()->pfc.fifoX++;
    }
  }

  return true;
}


void pipelineFetch() 
{
  switch(ppuGetContext()->pfc.curFetchState) 
  {
    case FS_TILE: 
      {
        ppuGetContext()->fetchedEntryCount = 0;

        if (LCD_CNTRL_BGW_ENABLE) 
        {
          ppuGetContext()->pfc.bgFetchData[0] = 
            busRead(LCD_CNTRL_BG_MAP_AREA             + 
                   (ppuGetContext()->pfc.mapX   / 8)  + 
                   (((ppuGetContext()->pfc.mapY / 8)) * 32));
        
          if (LCD_CNTRL_BGW_DATA_AREA == 0x8800) ppuGetContext()->pfc.bgFetchData[0] += 128;
        }

        pipelineLoadWindowTile();

        if (LCD_CNTRL_OBJ_ENABLE && ppuGetContext()->lineSprites) pipeleinLoadSpriteTile();

        ppuGetContext()->pfc.curFetchState = FS_DATA0;
        ppuGetContext()->pfc.fetchX       += 8;
        break;
      } 

    case FS_DATA0: 
      {
        ppuGetContext()->pfc.bgFetchData[1] = 
          busRead(LCD_CNTRL_BGW_DATA_AREA                   +
                  (ppuGetContext()->pfc.bgFetchData[0] * 16) + 
                  ppuGetContext()->pfc.tileY);

        pipeleinLoadSpriteData(0);

        ppuGetContext()->pfc.curFetchState = FS_DATA1;
        break;
      } 

    case FS_DATA1: 
      {
        ppuGetContext()->pfc.bgFetchData[2] = 
          busRead(LCD_CNTRL_BGW_DATA_AREA                   +
                 (ppuGetContext()->pfc.bgFetchData[0] * 16) + 
                 ppuGetContext()->pfc.tileY + 1);

        pipeleinLoadSpriteData(1);

        ppuGetContext()->pfc.curFetchState = FS_IDLE;
        break;
      }

    case FS_IDLE: 
      {
        ppuGetContext()->pfc.curFetchState = FS_PUSH;
        break;
      }

    case FS_PUSH: 
      {
        if (pipelineFifoAdd()) ppuGetContext()->pfc.curFetchState = FS_TILE;
        break;
      } 
  }
}

void pipelinePushPixel() 
{
  if (ppuGetContext()->pfc.pixelFifo.size > 8) 
  {
    u32 pixelData = pixelFifoPop();

    if (ppuGetContext()->pfc.lineX >= (lcdGetContext()->scrollX % 8)) 
    {
      ppuGetContext()->frameBuffer[
        ppuGetContext()->pfc.pushedX + 
        (lcdGetContext()->ly * XRES)] = pixelData;

      ppuGetContext()->pfc.pushedX++;
    }

    ppuGetContext()->pfc.lineX++;
  }
}

void pipelineProcess() 
{
  ppuGetContext()->pfc.mapY  = (lcdGetContext()->ly + lcdGetContext()->scrollY);
  ppuGetContext()->pfc.mapX  = (ppuGetContext()->pfc.fetchX + lcdGetContext()->scrollX);
  ppuGetContext()->pfc.tileY = ((lcdGetContext()->ly + lcdGetContext()->scrollY) % 8) * 2;

  if (!(ppuGetContext()->lineTicks & 1)) pipelineFetch();

  pipelinePushPixel();
}

void pipelineFifoReset() 
{
  while(ppuGetContext()->pfc.pixelFifo.size) pixelFifoPop();
  ppuGetContext()->pfc.pixelFifo.head = 0;
}
