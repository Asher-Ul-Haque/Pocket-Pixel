#include <cartridge/cartridge.h>
#include <ppu/ppu.h>
#include <ppu/ppuRegisters.h>
#include <ppu/internal.h>
#include <utils/bitwise.h>

/**
 * @file ppuPipeline.c
 * @brief Cycle-accurate Pixel FIFO and Tile Fetcher.
 * Modeling the SM83 PPU fetcher state machine.
*/

void fifoPush(PpuFifo* FIFO, PpuPixel PIXEL) 
{
  FIFO->size++;
  FIFO->pixels[FIFO->tail] = PIXEL;
  FIFO->tail               = (FIFO->tail + 1) & FIFO_MASK;
}

PpuPixel fifoPop(PpuFifo* FIFO) 
{
  PpuPixel pixel = FIFO->pixels[FIFO->head];
  FIFO->head     = (FIFO->head + 1) & FIFO_MASK;
  FIFO->size--;
  return pixel;
}


// - - - Fetcher Logic - - -

static void fetcherStep(void)
{
  PpuContext* ctx     = ppuGetContext();
  PpuFetcher* fetcher = &ctx->fetcher;

  // - - - Fetcher operates at 1/2 speed (ticks every 2 T-cycles)
  if (ctx->dotClock % T_CYCLE_STEP != 0) return;

  switch (fetcher->state) 
  {
    case FETCH_GET_TILE: 
      {
        // - - - Determine Tile Map Address (BG or Window)
        u16 mapBase  = ctx->windowTriggered ? LCDC_WIN_MAP(ctx) : LCDC_BG_MAP(ctx);
        u8  tileY    = ctx->windowTriggered ? 
                      (ctx->windowLineCounter / TILE_PIXEL_WIDTH) : 
                     ((ctx->ly + ctx->scy) / TILE_PIXEL_WIDTH);
        u8  tileX    = fetcher->xOffset; // - - - Incremented per 8 pixels
        u16 addr     = (mapBase - VRAM_START) + (tileY * MAP_ROW_SIZE_TILES) + (tileX & (MAP_ROW_SIZE_TILES - 1));

        fetcher->tileIndex = ctx->vram[0][addr];
        
        // - - - In CGB mode, attributes are fetched from Bank 1 at the same address
        fetcher->tileAttr = ctx->vram[1][addr]; 
        fetcher->state    = FETCH_GET_TILE_LOW;
        break;
      }

    case FETCH_GET_TILE_LOW: 
      {
        // - - - Calculate Tile Data Address based on LCDC Bit 4 (Addressing Mode)
        u8 offset_y = ctx->windowTriggered ? 
                     (ctx->windowLineCounter % TILE_PIXEL_WIDTH) : 
                    ((ctx->ly + ctx->scy) % TILE_PIXEL_WIDTH);
        
        // - - - Handle Vertical Flip (CGB only)
        if (ATTR_V_FLIP(fetcher->tileAttr)) offset_y = (TILE_PIXEL_WIDTH - 1) - offset_y;

        u16 tileDataBase = LCDC_DATA_AREA(ctx);
        u16 addr;

        if (tileDataBase == VRAM_TILE_DATA_0_ADDR) 
        {
          // - - - Signed addressing mode (0x8800-0x97FF)
          addr = tileDataBase + ((i8)fetcher->tileIndex + TILE_DATA_SIGNED_OFFSET) * TILE_DATA_SIZE_BYTES + (offset_y * 2);
        } 
        else 
        {
          // - - - Unsigned addressing mode (0x8000-0x8FFF)
          addr = tileDataBase + (fetcher->tileIndex * TILE_DATA_SIZE_BYTES) + (offset_y * 2);
        }

        // - - - Determine VRAM bank for tile data (CGB attribute bit 3)
        u8 bank          = ATTR_VRAM_BANK(fetcher->tileAttr);
        fetcher->dataLow = ctx->vram[bank][addr & VRAM_MASK];
        fetcher->state   = FETCH_GET_TILE_HIGH;
        break;
      }

    case FETCH_GET_TILE_HIGH: 
      {
        u8 offsetY = ctx->windowTriggered ? 
                    (ctx->windowLineCounter % TILE_PIXEL_WIDTH) : 
                   ((ctx->ly + ctx->scy) % TILE_PIXEL_WIDTH);
        if (ATTR_V_FLIP(fetcher->tileAttr)) offsetY = (TILE_PIXEL_WIDTH - 1) - offsetY;

        u16 tileDataBase = LCDC_DATA_AREA(ctx);
        u16 addr         = (tileDataBase == VRAM_TILE_DATA_0_ADDR) ? 
            (tileDataBase + ((i8)fetcher->tileIndex + TILE_DATA_SIGNED_OFFSET) * TILE_DATA_SIZE_BYTES + (offsetY * 2) + 1) :
            (tileDataBase + (fetcher->tileIndex * TILE_DATA_SIZE_BYTES) + (offsetY * 2) + 1);

        u8 bank             = ATTR_VRAM_BANK(fetcher->tileAttr);
        fetcher->dataHigh   = ctx->vram[bank][addr & 0x1FFF];
        fetcher->state      = FETCH_PUSH;
        break;
      }

    case FETCH_PUSH:
      {
        // - - - Push 8 pixels into BG FIFO if it has space
        if (ctx->bgFifo.size <= TILE_PIXEL_WIDTH) 
        {
          for (i32 i = (TILE_PIXEL_WIDTH - 1); i >= 0; i--) 
          {
            // - - - Handle Horizontal Flip (CGB only)
            i32 bit = ATTR_H_FLIP(fetcher->tileAttr) ? ((TILE_PIXEL_WIDTH - 1) - i) : i;
            
            u8 low      = (fetcher->dataLow  >> bit) & 1;
            u8 high     = (fetcher->dataHigh >> bit) & 1;
            u8 colorID  = (high << 1) | low;

            PpuPixel p = 
            {
              .pixel        = colorID,
              .palette      = ATTR_CGB_PAL(fetcher->tileAttr),
              .bgPriority   = ATTR_PRIORITY(fetcher->tileAttr),
              .priority     = 0
            };
            fifoPush(&ctx->bgFifo, p);
          }
          fetcher->xOffset++;
          fetcher->state = FETCH_GET_TILE;
        }
        break;
      }
            
    default: break;
  }
}


// - - - Main Pipeline Logic - - -

void ppuPipelineTick(void) 
{
  PpuContext* ctx = ppuGetContext();

  // - - - 1. Advance the Fetcher
  fetcherStep();

  // - - - 2. Window Check: Trigger window if LY matches WY and WX matches current X
  if (LCDC_WIN_ENABLED(ctx) && !ctx->windowTriggered) 
  {
    if (ctx->ly >= ctx->wy && (ctx->pixelsPushed + (TILE_PIXEL_WIDTH - 1)) >= ctx->wx) 
    {
      ctx->windowTriggered = true;
      ctx->fetcher.state   = FETCH_GET_TILE;  // - - - Reset fetcher for window tile
      ctx->fetcher.xOffset = 0;
      ctx->bgFifo.size     = 0;               // - - - Clear FIFO to restart with window pixels
    }
  }

  // - - - 3. FIFO Pop & Pixel Push. The FIFO must contain more than 8 pixels to begin pushing to screen
  if (ctx->bgFifo.size > TILE_PIXEL_WIDTH) 
  {
    PpuPixel p = fifoPop(&ctx->bgFifo);

    // - - - Fine SCX Scrolling: Discard pixels until alignment is reached
    if (ctx->scrollXFifoAdj > 0) 
    {
      ctx->scrollXFifoAdj--;
      return;
    }

    // - - - Logic for DMG vs CGB color selection
    u32 color;
    if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY) color = ppuGetColorDMG(ctx->bgp, p.pixel);
    else                                                 color = ppuGetColorCGB(p.palette, p.pixel, false);

    // - - - Write to Frame Buffer
    ctx->frameBuffer[ctx->pixelsPushed + (ctx->ly * SCREEN_WIDTH)] = color;
    ctx->pixelsPushed++;
  }
}
