#include <cartridge/cartridge.h>
#include <ppu/ppu.h>
#include <ppu/ppuRegisters.h>
#include <ppu/internal.h>
#include <utils/bitwise.h>
#include <ppu/oam.h>

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
  bool        dmg = (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY);

  // - - - 1. Advance the Fetcher and check for sprites 
  fetcherStep();
  ppuOverlayDelaySprites();

  // - - - 2. Window Check: Trigger window if LY matches WY and WX matches current X
  if (LCDC_WIN_ENABLED(ctx) && !ctx->windowTriggered) 
  {
    if (ctx->ly >= ctx->wy && (ctx->pixelsPushed + (TILE_PIXEL_WIDTH - 1)) >= (ctx->wx - WINDOW_X_REG_BIAS)) 
    {
      ctx->windowTriggered = true;
      ctx->fetcher.state   = FETCH_GET_TILE;  // - - - Reset fetcher for window tile
      ctx->fetcher.xOffset = 0;
      ctx->bgFifo.size     = 0;               // - - - Clear FIFO to restart with window pixels
      
      // Debug logging (remove in production)
      static int windowTriggerCount = 0;
      if (windowTriggerCount++ < 10)
      {
        FORGE_LOG_DEBUG("Window triggered at LY=%d, pixelsPushed=%d, WY=%d, WX=%d (adjusted: %d)", 
                       ctx->ly, ctx->pixelsPushed, ctx->wy, ctx->wx, ctx->wx - WINDOW_X_REG_BIAS);
      }
    }
  }

  // - - - 3. FIFO Pop & Pixel Push. The FIFO must contain more than 8 pixels to begin pushing to screen
  if (ctx->bgFifo.size > TILE_PIXEL_WIDTH) 
  {
    PpuPixel bgPixel = fifoPop(&ctx->bgFifo);

    // - - - Fine SCX Scrolling: Discard pixels until alignment is reached
    if (ctx->scrollXFifoAdj > 0) 
    {
      ctx->scrollXFifoAdj--;
      return;
    }

    // - - - Logic for DMG vs CGB color selection
    u32      color    = 0;
    PpuPixel objPixel = 
      {
        .pixel = 0 
      };
    bool hasSprite = false;

    // - - - Pop a sprite pixel if available
    if (ctx->objFifo.size > 0)
    {
      objPixel = fifoPop(&ctx->objFifo);
      if (objPixel.pixel != 0 && LCDC_OBJ_ENABLED(ctx)) hasSprite = true;
    }

    /**
      * Composition Rules
      * 1. If no sprite pixel, use BG pixel color.
      * 2. If sprite is present and BG pixel is color 0, use sprite pixel color.
      * 3. If sprite is present and has higher priority than BG, use sprite pixel color.
      * 4. Otherwise, use BG pixel color.
    */
    if (hasSprite && ((bgPixel.pixel == 0) || !objPixel.bgPriority))
    {
      if (dmg) 
      {
        u8 pal = (objPixel.palette == 0) ? ctx->obp0 : ctx->obp1;
        color = ppuGetColorDMG(pal, objPixel.pixel);
      }
      else 
      {
        color = ppuGetColorCGB(objPixel.palette, objPixel.pixel, true);
      }
    }
    else 
    {
      if (dmg)  color = ppuGetColorDMG(ctx->bgp, bgPixel.pixel);
      else      color = ppuGetColorCGB(bgPixel.palette, bgPixel.pixel, false);
    }

     // - - - Write to Frame Buffer
    ctx->frameBuffer[ctx->pixelsPushed + (ctx->ly * SCREEN_WIDTH)] = color;
    
    // Debug logging for sprite rendering (first occurrence only)
    static int spriteRenderCount = 0;
    if (hasSprite && spriteRenderCount++ < 5)
    {
      FORGE_LOG_DEBUG("Sprite pixel rendered at LY=%d, X=%d, color=%d, bgPixel=%d, priority=%d", 
                     ctx->ly, ctx->pixelsPushed, objPixel.pixel, bgPixel.pixel, objPixel.bgPriority);
    }
    
    ctx->pixelsPushed++;
  }
}
