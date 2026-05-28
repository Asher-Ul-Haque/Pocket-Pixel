#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cartridge/cartridge.h>

void ppuResetFetcher(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->fetcher.state          = FETCH_STATE_GET_TILE_MAP;
  ctx->fetcher.stepClock      = FETCH_CLOCK_RESET;
  ctx->fetcher.tileMapIndex   = OAM_DMA_START_INDEX;
  ctx->fetcher.tileAttributes = OAM_DMA_START_INDEX;
  ctx->fetcher.tileDataLow    = OAM_DMA_START_INDEX;
  ctx->fetcher.tileDataHigh   = OAM_DMA_START_INDEX;
  ctx->fetcher.fetcherX       = OAM_DMA_START_INDEX;
}

void ppuStepPixelFetcher(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->fetcher.stepClock++;
  if (ctx->fetcher.stepClock < FETCH_STEP_DOTS) return;
  ctx->fetcher.stepClock = FETCH_CLOCK_RESET;

  bool  windowEnabled         = (ctx->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;
  int   windowActivationEdge  = (int) ctx->registers.wx - WINDOW_X_OFFSET;

  // Fix: Check ctx->screenX instead of fetcherX so it aligns visually with the display
  if (windowEnabled && ctx->registers.ly >= ctx->registers.wy && (int)ctx->screenX >= windowActivationEdge)
  {
    if (!ctx->windowTriggered)
    {
      ctx->windowTriggered  = true;
      ctx->fetcher.state    = FETCH_STATE_GET_TILE_MAP;
      ctx->fetcher.fetcherX = 0;

      // Fix: Flush FIFOs instantly to tear off trailing BG pixels
      ctx->bgFifo.count  = FIFO_EMPTY_COUNT;
      ctx->bgFifo.head   = FIFO_EMPTY_COUNT;
      ctx->bgFifo.tail   = FIFO_EMPTY_COUNT;
      ctx->objFifo.count = FIFO_EMPTY_COUNT;
      ctx->objFifo.head  = FIFO_EMPTY_COUNT;
      ctx->objFifo.tail  = FIFO_EMPTY_COUNT;
    }
  }

  switch (ctx->fetcher.state)
  {
    case FETCH_STATE_GET_TILE_MAP:
      {
        u16 tileMapBaseAddress  = MAP_START_0;
        u16 finalMapOffset      = 0;

        if (!ctx->windowTriggered)
        {
          if ((ctx->registers.lcdc & LCDC_BG_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }

          u16 tileY       = ((ctx->registers.ly + ctx->registers.scy) / TILE_SIDE) % TILE_MAP_WIDTH;
          u16 tileX       = (((ctx->registers.scx / TILE_SIDE) + ctx->fetcher.fetcherX) % TILE_MAP_WIDTH);
          finalMapOffset  = (tileY * TILE_MAP_WIDTH) + tileX;
        }
        else
        {
          if ((ctx->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }

          u16 tileY       = ((u16)ctx->windowLineCounter / TILE_SIDE) % TILE_MAP_WIDTH;
          u16 tileX       = ctx->fetcher.fetcherX % TILE_MAP_WIDTH;
          finalMapOffset  = (tileY * TILE_MAP_WIDTH) + tileX;
        }

        u16 localVramIndex = (tileMapBaseAddress - VRAM_START_ADDR) + finalMapOffset;

        ctx->fetcher.tileMapIndex = ctx->vram[DEFAULT_VRAM_BANK][localVramIndex];
        ctx->fetcher.state        = FETCH_STATE_GET_TILE_ATTR;
        break;
      }

    case FETCH_STATE_GET_TILE_ATTR:
      {
        if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY)
        {
          ctx->fetcher.tileAttributes = OAM_DMA_START_INDEX;
          ctx->fetcher.state          = FETCH_STATE_GET_TILE_DATA_L;
          break;
        }

        u16 tileMapBaseAddress = MAP_START_0;
        if (!ctx->windowTriggered)
        {
          if ((ctx->registers.lcdc & LCDC_BG_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }
        }
        else
        {
          if ((ctx->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }
        }

        u16 tileY = 0;
        u16 tileX = 0;
        if (!ctx->windowTriggered)
        {
          tileY = ((ctx->registers.ly + ctx->registers.scy) / TILE_SIDE) % TILE_MAP_WIDTH;
          tileX = (((ctx->registers.scx / TILE_SIDE) + ctx->fetcher.fetcherX) % TILE_MAP_WIDTH);
        }
        else
        {
          tileY = ((u16)ctx->windowLineCounter / TILE_SIDE) % TILE_MAP_WIDTH;
          tileX = ctx->fetcher.fetcherX % TILE_MAP_WIDTH;
        }
        u16 finalMapOffset = (tileY * TILE_MAP_WIDTH) + tileX;
        u16 localVramIndex = (tileMapBaseAddress - VRAM_START_ADDR) + finalMapOffset;

        ctx->fetcher.tileAttributes = ctx->vram[BUS_BANK_BIT_MASK][localVramIndex];
        ctx->fetcher.state          = FETCH_STATE_GET_TILE_DATA_L;
        break;
      }

    case FETCH_STATE_GET_TILE_DATA_L:
      {
        u8 tileRow = (ctx->registers.ly + ctx->registers.scy) % TILE_SIDE;
        if (ctx->windowTriggered)
        { tileRow = ctx->windowLineCounter % TILE_SIDE; }

        if ((ctx->fetcher.tileAttributes & ATTR_Y_FLIP_MASK) != 0)
        { tileRow = (TILE_SIDE - PIXEL_COLOR_MASK) - tileRow; }

        u16 tileDataAddress = 0;

        if ((ctx->registers.lcdc & LCDC_BG_WIN_DATA_MASK) != 0)
        { tileDataAddress = TILE_DATA_MODE_1_START + ((u16)ctx->fetcher.tileMapIndex * TILE_BYTES); }
        else
        {
          i16 signedIndex = (i8) ctx->fetcher.tileMapIndex;
          tileDataAddress = (u16)((i32) TILE_DATA_MODE_0_START + (signedIndex * TILE_BYTES));
        }

        u8  chosenVramBank = (ctx->fetcher.tileAttributes & ATTR_VRAM_BANK_MASK) ? BUS_BANK_BIT_MASK : DEFAULT_VRAM_BANK;
        u16 targetedOffset = (tileDataAddress + ((u16)tileRow * TILE_LINE_BYTES) - VRAM_START_ADDR) & (VRAM_BANK_SIZE - PIXEL_COLOR_MASK);

        ctx->fetcher.tileDataLow  = ctx->vram[chosenVramBank][targetedOffset];
        ctx->fetcher.state        = FETCH_STATE_GET_TILE_DATA_H;
        break;
      }

    case FETCH_STATE_GET_TILE_DATA_H:
      {
        u8 tileRow = (ctx->registers.ly + ctx->registers.scy) % TILE_SIDE;
        if (ctx->windowTriggered)
        { tileRow = ctx->windowLineCounter % TILE_SIDE; }

        if ((ctx->fetcher.tileAttributes & ATTR_Y_FLIP_MASK) != 0)
        { tileRow = (TILE_SIDE - PIXEL_COLOR_MASK) - tileRow; }

        u16 tileDataAddress = 0;
        if ((ctx->registers.lcdc & LCDC_BG_WIN_DATA_MASK) != 0)
        { tileDataAddress = TILE_DATA_MODE_1_START + ((u16)ctx->fetcher.tileMapIndex * TILE_BYTES); }
        else
        {
          i16 signedIndex = (i8) ctx->fetcher.tileMapIndex;
          tileDataAddress = (u16)((i32) TILE_DATA_MODE_0_START + (signedIndex * TILE_BYTES));
        }

        u8  chosenVramBank = (ctx->fetcher.tileAttributes & ATTR_VRAM_BANK_MASK) ? BUS_BANK_BIT_MASK : DEFAULT_VRAM_BANK;
        u16 targetedOffset = (tileDataAddress + ((u16)tileRow * TILE_LINE_BYTES) + PIXEL_COLOR_MASK - VRAM_START_ADDR) & (VRAM_BANK_SIZE - PIXEL_COLOR_MASK);

        ctx->fetcher.tileDataHigh = ctx->vram[chosenVramBank][targetedOffset];
        ctx->fetcher.state        = FETCH_STATE_PUSH_TO_FIFO;
        break;
      }

    case FETCH_STATE_PUSH_TO_FIFO:
      {
        if (ppuPushTileToFifo())
        {
          ctx->fetcher.fetcherX++;
          ctx->fetcher.state = FETCH_STATE_GET_TILE_MAP;
        }
        else 
        {
          ctx->fetcher.stepClock = 0;
        }
        break;
      }
  }
}
