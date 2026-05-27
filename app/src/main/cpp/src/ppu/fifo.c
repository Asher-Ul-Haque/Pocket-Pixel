#include <cartridge/cartridge.h>
#include <ppu/ppu.h>
#include <ppu/internal.h>

void ppuResetFifos(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->bgFifo.count = FIFO_EMPTY_COUNT;
  ctx->bgFifo.head  = FIFO_EMPTY_COUNT;
  ctx->bgFifo.tail  = FIFO_EMPTY_COUNT;

  ctx->objFifo.count = FIFO_EMPTY_COUNT;
  ctx->objFifo.head  = FIFO_EMPTY_COUNT;
  ctx->objFifo.tail  = FIFO_EMPTY_COUNT;

  for (int i = 0; i < FIFO_CAPACITY; i++)
  {
    ctx->objFifo.pixels[i].colorIndex = PIXEL_COLOR_TRANSPARENT;
  }

  ctx->droppedPixels  = FIFO_EMPTY_COUNT;
  ctx->screenX        = FIFO_EMPTY_COUNT;
  ctx->spriteFetching = false;
}

bool ppuPushTileToFifo(void)
{
  PpuContext* ctx = ppuGetContext();

  if (ctx->bgFifo.count > FIFO_EMPTY_COUNT) return false;

  // - - - Check if the background tile has the X-Flip attribute enabled
  bool bgXFlip = (ctx->fetcher.tileAttributes & ATTR_X_FLIP_MASK) != 0;

  for (u8 pixelIndex = FIFO_EMPTY_COUNT; pixelIndex < PIXEL_BIT_WIDTH; ++pixelIndex)
  {
    // - - - If X-Flip is true, parse from right-to-left instead of left-to-right
    u8 bitShift = bgXFlip ? pixelIndex : ((PIXEL_BIT_WIDTH - BIT_MASK_BASE) - pixelIndex);
    u8 bitMask  = (BIT_MASK_BASE << bitShift);

    u8 colorBitLow  = (ctx->fetcher.tileDataLow  & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 colorBitHigh = (ctx->fetcher.tileDataHigh & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;

    u8 finalColorIndex = (colorBitHigh << PIXEL_SHIFT_HIGH_BIT) | colorBitLow;

    PpuPixel bgPixel = 
      {
        .colorIndex     = finalColorIndex,
        .palette        = ctx->fetcher.tileAttributes & ATTR_PALETTE_MASK,
        .bgPriority     = (ctx->fetcher.tileAttributes & ATTR_PRIORITY_MASK) != 0,
        .spritePriority = false
      };

    ctx->bgFifo.pixels[ctx->bgFifo.tail] = bgPixel;
    
    ctx->bgFifo.tail  = (ctx->bgFifo.tail + BIT_MASK_BASE) % FIFO_CAPACITY;
    ctx->objFifo.tail = (ctx->objFifo.tail + BIT_MASK_BASE) % FIFO_CAPACITY;
    
    ctx->bgFifo.count++;
    ctx->objFifo.count++;
  }
  return true;
}

void ppuInjectSpriteToFifo(u8 SPRITE_LINE_BUFFER_INDEX)
{
  PpuContext* ctx = ppuGetContext();

  u8  oamIndex        = ctx->scanlineOamBuffer.spriteIndices[SPRITE_LINE_BUFFER_INDEX];
  u16 oamBaseAddress  = (u16) oamIndex * SPRITE_OAM_ENTRY_BYTES;

  u8 spriteY    = ctx->oam[oamBaseAddress + OAM_Y_OFFSET];
  u8 spriteX    = ctx->oam[oamBaseAddress + OAM_X_OFFSET];
  u8 tileIndex  = ctx->oam[oamBaseAddress + OAM_TILE_OFFSET];
  u8 attributes = ctx->oam[oamBaseAddress + OAM_ATTR_OFFSET];

  u8 spriteHeight = ((ctx->registers.lcdc & LCDC_OBJ_SIZE_MASK) != 0) ? SPRITE_HEIGHT_16 : SPRITE_HEIGHT_8;
  u8 lineOffset = (ctx->registers.ly + SPRITE_Y_OFFSET) - spriteY;

  if ((attributes & OBJ_ATTR_Y_FLIP_MASK) != 0)
  { lineOffset = (spriteHeight - BIT_MASK_BASE) - lineOffset; }

  if (spriteHeight == SPRITE_HEIGHT_16)
  {
    tileIndex &= SPRITE_TILE_MASK_8X16;
    if (lineOffset >= SPRITE_HEIGHT_8) tileIndex |= BIT_MASK_BASE;
  }

  u8  tileRow = lineOffset % TILE_SIDE;
  u16 localVramOffset = ((u16) tileIndex * TILE_BYTES) + ((u16) tileRow * TILE_LINE_BYTES);

  u8 vramBank       = (attributes & OBJ_ATTR_VRAM_BANK_MASK) ? BUS_BANK_BIT_MASK : DEFAULT_VRAM_BANK;
  u8 spriteDataLow  = ctx->vram[vramBank][localVramOffset];
  u8 spriteDataHigh = ctx->vram[vramBank][localVramOffset + BIT_MASK_BASE];

  i16 screenOffset = (i16)spriteX - 8 - (i16)ctx->screenX;

  for (u8 pixelStep = FIFO_EMPTY_COUNT; pixelStep < PIXEL_BIT_WIDTH; pixelStep++)
  {
    i16 drawX = screenOffset + pixelStep;
    
    if (drawX < 0 || drawX >= 8) continue; 

    u8 bitShift = ((attributes & OBJ_ATTR_X_FLIP_MASK) != 0) ? pixelStep : ((PIXEL_BIT_WIDTH - BIT_MASK_BASE) - pixelStep);
    u8 bitMask  = (BIT_MASK_BASE << bitShift);

    u8 colorBitLow      = (spriteDataLow  & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 colorBitHigh     = (spriteDataHigh & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 finalColorIndex  = (colorBitHigh << PIXEL_SHIFT_HIGH_BIT) | colorBitLow;

    if (finalColorIndex == PIXEL_COLOR_TRANSPARENT) continue;

    u8 targetFifoSlot = (ctx->objFifo.head + drawX) % FIFO_CAPACITY;

    if (ctx->objFifo.pixels[targetFifoSlot].colorIndex != PIXEL_COLOR_TRANSPARENT) continue;

    PpuPixel spritePixel;
    spritePixel.colorIndex = finalColorIndex;

    if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY)
    {
      spritePixel.palette = (attributes & OBJ_ATTR_DMG_PALETTE_MASK) ? DMG_OBJ_PALETTE_1 : DMG_OBJ_PALETTE_0;
    }
    else 
    {
      spritePixel.palette = attributes & OBJ_ATTR_CGB_PALETTE_MASK;
    }

    spritePixel.spritePriority = (attributes & OBJ_ATTR_BG_PRIORITY_MASK) != 0;
    spritePixel.bgPriority     = false;

    ctx->objFifo.pixels[targetFifoSlot] = spritePixel;
  }
}
