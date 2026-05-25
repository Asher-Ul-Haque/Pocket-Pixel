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

  // - - - advance the internal 2-dot step division clock 
  ctx->fetcher.stepClock++;
  if (ctx->fetcher.stepClock < FETCH_STEP_DOTS) return;
  ctx->fetcher.stepClock = FETCH_CLOCK_RESET;

  // - - - Hardware window retargeting intercept 
  bool  windowEnabled         = (ctx->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;
  u16   currentScreenX        = (u16) ctx->fetcher.fetcherX * TILE_SIDE;
  u16   windowActivationEdge  = (u16) ctx->registers.wx - WINDOW_X_OFFSET;

  if (windowEnabled && ctx->registers.ly >= ctx->registers.wy && currentScreenX >= windowActivationEdge)
  {
    if (!ctx->windowTriggered)
    {
      ctx->windowTriggered  = true;
      ctx->fetcher.state    = FETCH_STATE_GET_TILE_MAP;
    }
  }

  // - - - Process the active execution state step 
  switch (ctx->fetcher.state)
  {
    case FETCH_STATE_GET_TILE_MAP:
      {
        u16 tileMapBaseAddress  = MAP_START_0;
        u16 finalMapOffset      = 0;

        if (!ctx->windowTriggered)
        {
          // - - - Background map selection based on LCDC Bit 3
          if ((ctx->registers.lcdc & LCDC_BG_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }

          // - - - Calculate background horizontal/vertical tile map layout offsets
          u16 tileY       = ((ctx->registers.ly + ctx->registers.scy) / TILE_SIDE) % TILE_MAP_WIDTH;
          u16 tileX       = (((ctx->registers.scx / TILE_SIDE) + ctx->fetcher.fetcherX) % TILE_MAP_WIDTH);
          finalMapOffset  = (tileY * TILE_MAP_WIDTH) + tileX;
        }
        else
        {
          // - - - Window map selection based on LCDC Bit 6
          if ((ctx->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) != 0)
          { tileMapBaseAddress = MAP_START_1; }

          // - - -  Calculate window horizontal/vertical tile map layout offsets using the hidden counter
          u16 tileY       = ((u16)ctx->windowLineCounter / TILE_SIDE) % TILE_MAP_WIDTH;
          u16 tileX       = ctx->fetcher.fetcherX % TILE_MAP_WIDTH;
          finalMapOffset  = (tileY * TILE_MAP_WIDTH) + tileX;
        }

        // - - - Correct Indexing: Resolve base map destination address minus VRAM system offset
        u16 localVramIndex = (tileMapBaseAddress - VRAM_START_ADDR) + finalMapOffset;

        ctx->fetcher.tileMapIndex = ctx->vram[DEFAULT_VRAM_BANK][localVramIndex];
        ctx->fetcher.state        = FETCH_STATE_GET_TILE_ATTR;
        break;
      }

    case FETCH_STATE_GET_TILE_ATTR:
      {
        // - - - DMG Mode ignores attributes; step instantly into tile data calculations
        if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY)
        {
          ctx->fetcher.tileAttributes = OAM_DMA_START_INDEX;
          ctx->fetcher.state          = FETCH_STATE_GET_TILE_DATA_L;
          break;
        }

        // - - - GBC Mode calculates the exact same map offset to pull attributes from VRAM Bank 1
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

        // - - - Fetch attribute properties byte directly out of the secondary CGB VRAM bank layer
        ctx->fetcher.tileAttributes = ctx->vram[BUS_BANK_BIT_MASK][localVramIndex];
        ctx->fetcher.state          = FETCH_STATE_GET_TILE_DATA_L;
        break;
      }

    case FETCH_STATE_GET_TILE_DATA_L:
      {
        // - - - Determine active vertical row line index inside the targeted tile
        u8 tileRow = (ctx->registers.ly + ctx->registers.scy) % TILE_SIDE;
        if (ctx->windowTriggered)
        { tileRow = ctx->windowLineCounter % TILE_SIDE; }

        // - - - Apply GBC vertical Y-Flip mask modification if configured
        if ((ctx->fetcher.tileAttributes & ATTR_Y_FLIP_MASK) != 0)
        { tileRow = (TILE_SIDE - PIXEL_COLOR_MASK) - tileRow; }

        u16 tileDataAddress = 0;

        // - - - Addressing Mode 1 (LCDC Bit 4 = 1): Unsigned indexing at $8000
        if ((ctx->registers.lcdc & LCDC_BG_WIN_DATA_MASK) != 0)
        { tileDataAddress = TILE_DATA_MODE_1_START + ((u16)ctx->fetcher.tileMapIndex * TILE_BYTES); }

        // - - - Addressing Mode 0 (LCDC Bit 4 = 0): Signed indexing at $8800
        else
        {
          i16 signedIndex = (i8) ctx->fetcher.tileMapIndex;
          tileDataAddress = (u16)((i32) TILE_DATA_MODE_0_START + (signedIndex * TILE_BYTES));
        }

        // - - - Target the correct character data VRAM bank layer based on attribute preferences
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
          ctx->fetcher.stepClock = (FETCH_STEP_DOTS - PIXEL_COLOR_MASK);
        }
        break;
      }
  }
}
