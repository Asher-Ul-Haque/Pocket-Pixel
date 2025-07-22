#include "../include/ppu.h"
#include "../include/lcd.h"
#include "../include/bus.h"

void pixelFifoPush(u32 VALUE) 
{
  FifoEntry* next = (FifoEntry*) malloc(sizeof(FifoEntry));
  next->next = NULL;
  next->color = VALUE;

  // - - - first entry
  if (!ppuGetContext()->pfc.pixelFifo.head) ppuGetContext()->pfc.pixelFifo.head = ppuGetContext()->pfc.pixelFifo.tail = next;
  else 
  {
    ppuGetContext()->pfc.pixelFifo.tail->next = next;
    ppuGetContext()->pfc.pixelFifo.tail       = next;
  }

  ppuGetContext()->pfc.pixelFifo.size++;
}

u32 pixelFifoPop() 
{
  FORGE_ASSERT_MESSAGE(ppuGetContext()->pfc.pixelFifo.size > 0, "Error in pixel fifo");

  FifoEntry* popped                   = ppuGetContext()->pfc.pixelFifo.head;
  ppuGetContext()->pfc.pixelFifo.head = popped->next;
  ppuGetContext()->pfc.pixelFifo.size--;

  u32 val = popped->color;
  free(popped);

  return val;
}

u32 fetchSpritePixels(i32 BIT, u32 COLOR, u8 BG_COLOR) 
{
  for (i32 i = 0; i < ppuGetContext()->fetchedEntryCount; i++) 
  {
    i32 spX = (ppuGetContext()->fetchedEntries[i].x - 8) + ((lcdGetContext()->scrollX % 8));
    
    // - - - past that pixel already
    if (spX + 8 < ppuGetContext()->pfc.fifoX) continue;

    i32 offset = ppuGetContext()->pfc.fifoX - spX;

    // - - - out of bounds
    if (offset < 0 || offset > 7) continue;
    BIT = (7 - offset);

    if (ppuGetContext()->fetchedEntries[i].flagXflip) BIT = offset;

    u8   hi         = !!(ppuGetContext()->pfc.fetchEntryData[i * 2] & (1 << BIT));
    u8   lo         = !!(ppuGetContext()->pfc.fetchEntryData[(i * 2) + 1] & (1 << BIT)) << 1;
    bool bgPriority = ppuGetContext()->fetchedEntries[i].flagBGpallete;

    // - - - transparent
    if (!(hi|lo)) continue;

    if (!bgPriority || BG_COLOR == 0) 
    { 
      COLOR = (ppuGetContext()->fetchedEntries[i].flagPalleteNo) ? 
        lcdGetContext()->sp2Colors[hi|lo] : lcdGetContext()->sp1Colors[hi|lo];

      if (hi|lo) break;
    }
  }

  return COLOR;
}

bool pipelineFifoAdd() 
{
  // - - - fifo is full
  if (ppuGetContext()->pfc.pixelFifo.size > 8) return false;

  i32 x = ppuGetContext()->pfc.fetchX - (8 - (lcdGetContext()->scrollX % 8));

  for (i32 i = 0; i < 8; i++) 
  {
    i32 bit   = 7 - i;
    u8  hi    = !!(ppuGetContext()->pfc.bgFetchData[1] & (1 << bit));
    u8  lo    = !!(ppuGetContext()->pfc.bgFetchData[2] & (1 << bit)) << 1;
    u32 color = lcdGetContext()->bgColors[hi | lo];

    if (!LCD_CNTRL_BGW_ENABLE)  color = lcdGetContext()->bgColors[0];
    if (LCD_CNTRL_OBJ_ENABLE)   color = fetchSpritePixels(bit, color, hi | lo);

    if (x >= 0) 
    {
      pixelFifoPush(color);
      ppuGetContext()->pfc.fifoX++;
    }
  }

  return true;
}

void pipelineLoadSpriteTile() 
{
  oamLineEntry* le = ppuGetContext()->lineSprites;

  while(le) 
  {
    i32 sp_x = (le->entry.x - 8) + (lcdGetContext()->scrollX % 8);

    // - - - need to add entry
    if ((sp_x >= ppuGetContext()->pfc.fetchX && sp_x < ppuGetContext()->pfc.fetchX + 8) ||
       ((sp_x + 8) >= ppuGetContext()->pfc.fetchX && (sp_x + 8) < ppuGetContext()->pfc.fetchX + 8)) 
    { ppuGetContext()->fetchedEntries[ppuGetContext()->fetchedEntryCount++] = le->entry; }

    le = le->next;

    // - - - max checking three sprites on pixels
    if (!le || ppuGetContext()->fetchedEntryCount >= 3) break;
  }
}

void pipelineLoadSpriteData(u8 OFFSET) 
{
  i32 curY          = lcdGetContext()->ly;
  u8  spriteHeight  = LCD_CNTRL_OBJ_HEIGHT;

  for (i32 i = 0; i < ppuGetContext()->fetchedEntryCount; i++) 
  {
    u8 ty = ((curY + 16) - ppuGetContext()->fetchedEntries[i].y) * 2;

    // - - - flipped upside down
    if (ppuGetContext()->fetchedEntries[i].flagYflip) ty = ((spriteHeight * 2) - 2) - ty;

    u8 tileIndex = ppuGetContext()->fetchedEntries[i].tile;

    // - - - remove last bit
    if (spriteHeight == 16) tileIndex &= ~(1); 

    ppuGetContext()->pfc.fetchEntryData[(i * 2) + OFFSET] = 
      busRead(0x8000 + (tileIndex * 16) + ty + OFFSET);
  }
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
          ppuGetContext()->pfc.bgFetchData[0] 
            = busRead(LCD_CNTRL_BG_MAP_AREA   + 
              (ppuGetContext()->pfc.mapX / 8) + 
              (((ppuGetContext()->pfc.mapY / 8)) * 32));
            
          if (LCD_CNTRL_BGW_DATA_AREA == 0x8800) ppuGetContext()->pfc.bgFetchData[0] += 128;
        }

        if (LCD_CNTRL_OBJ_ENABLE && ppuGetContext()->lineSprites) pipelineLoadSpriteTile();

        ppuGetContext()->pfc.curFetchState = FS_DATA0;
        ppuGetContext()->pfc.fetchX       += 8;

        break;
      } 

      case FS_DATA0: 
      {
        ppuGetContext()->pfc.bgFetchData[1] 
          = busRead(LCD_CNTRL_BGW_DATA_AREA +
            (ppuGetContext()->pfc.bgFetchData[0] * 16) + 
            ppuGetContext()->pfc.tileY);

        pipelineLoadSpriteData(0);

        ppuGetContext()->pfc.curFetchState = FS_DATA1;
        break;
      } 

      case FS_DATA1: 
      {
        ppuGetContext()->pfc.bgFetchData[2] 
          = busRead(LCD_CNTRL_BGW_DATA_AREA            +
            (ppuGetContext()->pfc.bgFetchData[0] * 16) + 
            ppuGetContext()->pfc.tileY + 1);

        pipelineLoadSpriteData(1);

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
      ppuGetContext()->frameBuffer
        [ppuGetContext()->pfc.pushedX + (lcdGetContext()->ly * XRES)] = pixelData;

      ppuGetContext()->pfc.pushedX++;
    }

    ppuGetContext()->pfc.lineX++;
  }
}

void pipelineProcess() 
{
  ppuGetContext()->pfc.mapY   = (lcdGetContext()->ly + lcdGetContext()->scrollY);
  ppuGetContext()->pfc.mapX   = (ppuGetContext()->pfc.fetchX + lcdGetContext()->scrollX);
  ppuGetContext()->pfc.tileY  = ((lcdGetContext()->ly + lcdGetContext()->scrollY) % 8) * 2;

  if (!(ppuGetContext()->lineTicks & 1)) pipelineFetch();

  pipelinePushPixel();
}

void pipelineFifoReset() 
{
  while(ppuGetContext()->pfc.pixelFifo.size) pixelFifoPop();
  ppuGetContext()->pfc.pixelFifo.head = 0;
}
