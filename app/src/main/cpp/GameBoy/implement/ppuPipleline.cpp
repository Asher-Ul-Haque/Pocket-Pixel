#include "../include/ppu.h"
#include "../include/lcd.h"
#include "../include/bus.h"

void pixelFifoPush(u32 COLOR)
{
  FifoEntry* next   = (FifoEntry*) malloc(sizeof(FifoEntry));
  next->next        = NULL;
  next->color       = COLOR;

  if (!ppuGetContext()->pfc.pixelFifo.head)
  { ppuGetContext()->pfc.pixelFifo.head = next; }
  else 
  { ppuGetContext()->pfc.pixelFifo.tail->next = next; }

  ppuGetContext()->pfc.pixelFifo.tail = next;

  ppuGetContext()->pfc.pixelFifo.size++;
}

u32 pixelFifoPop()
{
  FORGE_ASSERT_MESSAGE(ppuGetContext()->pfc.pixelFifo.size > 0, "Erorr in pixel fifo");

  FifoEntry* pooped = ppuGetContext()->pfc.pixelFifo.head;
  ppuGetContext()->pfc.pixelFifo.head = pooped->next;
  ppuGetContext()->pfc.pixelFifo.size--;

  u32 value = pooped->color;
  free(pooped);

  return value;
}

bool pipelineFifoAdd()
{
  if (ppuGetContext()->pfc.pixelFifo.size > 8)
  {
    // - - - fifo is full
    return false;
  }

  int x = ppuGetContext()->pfc.fetchX - (8 - (lcdGetContext()->scrollX % 8));
  
  for (int i = 0; i < 8; ++i)
  {
    int bit   = 7 - i;
    u8  hi    = !!(ppuGetContext()->pfc.bgFetchData[1] & (1 << bit));
    u8  lo    = !!(ppuGetContext()->pfc.bgFetchData[2] & (1 << bit)) << 1;
    u32 color = lcdGetContext()->bgColors[hi | lo];

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
  switch(ppuGetContext()->pfc.currentFetchState)
  {
    case FS_TILE : 
      {
        if (LCD_CNTRL_BGW_ENABLE)
        {
          ppuGetContext()->pfc.bgFetchData[0] 
            = busRead(
              LCD_CNTRL_BG_MAP_AREA + 
              (ppuGetContext()->pfc.mapX / 8) + 
              (((ppuGetContext()->pfc.mapY / 8)) * 32));

          if (LCD_CNTRL_BGW_DATA_AREA == 0x8800)
          {
            ppuGetContext()->pfc.bgFetchData[0] += 128;
          }
        }

        ppuGetContext()->pfc.currentFetchState = FS_DATA_0;
        ppuGetContext()->pfc.fetchX           += 8;
        break;
      }

    case FS_IDLE :
      {
        ppuGetContext()->pfc.currentFetchState = FS_PUSH;
        break;
      }
    
    case FS_DATA_0 :
      {
        ppuGetContext()->pfc.bgFetchData[1] = 
          busRead(
            LCD_CNTRL_BGW_DATA_AREA                    + 
            (ppuGetContext()->pfc.bgFetchData[0] * 16) +
            ppuGetContext()->pfc.tileY
          );

        ppuGetContext()->pfc.currentFetchState = FS_DATA_1;
        break;
      }
    
    case FS_DATA_1 :
      {
        ppuGetContext()->pfc.bgFetchData[2] = 
          busRead(
            LCD_CNTRL_BGW_DATA_AREA                    + 
            (ppuGetContext()->pfc.bgFetchData[0] * 16) +
            ppuGetContext()->pfc.tileY + 1
          );

        ppuGetContext()->pfc.currentFetchState = FS_IDLE;
        break;
      }
  
    case FS_PUSH :
      {
        if (pipelineFifoAdd())
        {
          ppuGetContext()->pfc.currentFetchState = FS_TILE;
        }
        break;
      }
  }
}

void pipelinePushPixel()
{
  PPUcontext* ctx = ppuGetContext();
  if (ctx->pfc.pixelFifo.size > 8)
  {
    u32 pixelData = pixelFifoPop();

    if (ctx->pfc.lineX >= (lcdGetContext()->scrollX % 8))
    {
      ctx->frameBuffer[
        ctx->pfc.pushedX + 
        (lcdGetContext()->ly * X_RES)] = pixelData;

      ctx->pfc.pushedX++;
    }
    ctx->pfc.lineX++;
  }
}

void pipelineProc()
{
  ppuGetContext()->pfc.mapY  = (lcdGetContext()->ly + lcdGetContext()->scrollY);
  ppuGetContext()->pfc.mapX  = (ppuGetContext()->pfc.fetchX + lcdGetContext()->scrollX);
  ppuGetContext()->pfc.tileY = ((lcdGetContext()->ly + lcdGetContext()->scrollY) % 8) * 2; 

  if (!(ppuGetContext()->lineTicks & 1))
  {
    pipelineFetch();
  }

  pipelinePushPixel();
}

void pipelineFifoReset()
{
  while (ppuGetContext()->pfc.pixelFifo.size)
  { pixelFifoPop(); }

  ppuGetContext()->pfc.pixelFifo.head = 0;
}
