#include "cartridge/cartridge.h"
#include <ppu/ppu.h>
#include <ppu/internal.h>

void ppuResetFifos(void)
{
  PpuContext* ctx = ppuGetContext();

  // - - - Flush background FIFO 
  ctx->bgFifo.count = FIFO_EMPTY_COUNT;
  ctx->bgFifo.head  = FIFO_EMPTY_COUNT;
  ctx->bgFifo.tail  = FIFO_EMPTY_COUNT;

  // - - - Flush object FIFO 
  ctx->objFifo.count = FIFO_EMPTY_COUNT;
  ctx->objFifo.head  = FIFO_EMPTY_COUNT;
  ctx->objFifo.tail  = FIFO_EMPTY_COUNT;

  // - - - Reset layout alignment and injection trackers 
  ctx->droppedPixels  = FIFO_EMPTY_COUNT;
  ctx->screenX        = FIFO_EMPTY_COUNT;
  ctx->spriteFetching = false;
}

bool ppuPushTileToFifo(void)
{
  PpuContext* ctx = ppuGetContext();

  // - - - The fetcher can only push if the queue is completely empty 
  if (ctx->bgFifo.count > FIFO_EMPTY_COUNT) return false;

  // - - - Process all 8 pixels of the fetched tile row 
  for (u8 pixelIndex = FIFO_EMPTY_COUNT; pixelIndex < PIXEL_BIT_WIDTH; ++pixelIndex)
  {
    // - - - Gameboy rendering parses pixels left to right. Therefore, the leftmost pixel corressponds to Bit 7 (Most significatn bit) of the data bytes 
    u8 bitShift = (PIXEL_BIT_WIDTH - BIT_MASK_BASE) - pixelIndex;
    u8 bitMask  = (BIT_MASK_BASE << bitShift);

    // - - - Extract the target bit from both low and high VRAM data bytes 
    u8 colorBitLow  = (ctx->fetcher.tileDataLow  & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 colorBitHigh = (ctx->fetcher.tileDataHigh & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;

    // - - - Construct the final 2 bit color index 
    u8 finalColorIndex = (colorBitHigh << PIXEL_SHIFT_HIGH_BIT) | colorBitLow;

    // - - - Assemble the pixel 
    PpuPixel bgPixel = 
      {
        .colorIndex     = finalColorIndex,
        .palette        = ctx->fetcher.tileAttributes & ATTR_PALETTE_MASK,
        .bgPriority     = (ctx->fetcher.tileAttributes & ATTR_PRIORITY_MASK) != 0,
        .spritePriority = false
      };

    PpuPixel objPixel = 
      {
        .colorIndex     = PIXEL_COLOR_TRANSPARENT,
        .palette        = DEFAULT_BG_PALETTE,
        .bgPriority     = false,
        .spritePriority = false
      };

    // - - - Push the assembled pixels into their respective circular Fifo queues 
    ctx->bgFifo.pixels [ctx->bgFifo.tail]  = bgPixel;
    ctx->objFifo.pixels[ctx->objFifo.tail] = objPixel;
    ctx->bgFifo.count++;
    ctx->objFifo.count++;
  }
  return true;
}

void ppuInjectSpriteToFifo(u8 SPRITE_LINE_BUFFER_INDEX)
{
  PpuContext* ctx = ppuGetContext();

  // - - - Resolve the absolute OAM parameters from the cached line buffer 
  u8  oamIndex        = ctx->scanlineOamBuffer.spriteIndices[SPRITE_LINE_BUFFER_INDEX];
  u16 oamBaseAddress  = (u16) oamIndex * SPRITE_OAM_ENTRY_BYTES;

  u8 spriteY    = ctx->oam[oamBaseAddress + OAM_Y_OFFSET];
  u8 tileIndex  = ctx->oam[oamBaseAddress + OAM_TILE_OFFSET];
  u8 attributes = ctx->oam[oamBaseAddress + OAM_ATTR_OFFSET];

  // - - - Determine vertical height bounds 
  u8 spriteHeight = ((ctx->registers.lcdc & LCDC_OBJ_SIZE_MASK) != 0) ? SPRITE_HEIGHT_16 : SPRITE_HEIGHT_8;

  // - - - Calculate which vertical row of the sprite we are currently rendering 
  u8 lineOffset = (ctx->registers.ly + SPRITE_Y_OFFSET) - spriteY;

  if ((attributes & OBJ_ATTR_Y_FLIP_MASK) != 0)
  { lineOffset = (spriteHeight - BIT_MASK_BASE) - lineOffset; }

  // - - - 8x16 mode ignores Bit 0 of the tile index and forces it based on the row half 
  if (spriteHeight == SPRITE_HEIGHT_16)
  {
    tileIndex &= SPRITE_TILE_MASK_8X16;
    if (lineOffset >= SPRITE_HEIGHT_8) tileIndex |= BIT_MASK_BASE;
  }

  // - - - Calculate exact VRAM fetch addresses 
  u8  tileRow = lineOffset % TILE_SIDE;
  u16 localVramOffset = ((u16) tileIndex * TILE_BYTES) + ((u16) tileRow * TILE_LINE_BYTES);

  // - - - Select correct VRAM bank based on CGB attribute flags 
  u8 vramBank       = (attributes & OBJ_ATTR_VRAM_BANK_MASK) ? BUS_BANK_BIT_MASK : DEFAULT_VRAM_BANK;
  u8 spriteDataLow  = ctx->vram[vramBank][localVramOffset];
  u8 spriteDataHigh = ctx->vram[vramBank][localVramOffset + BIT_MASK_BASE];

  // - - - execute fifo overlay mixing 
  for (u8 pixelStep = FIFO_EMPTY_COUNT; pixelStep < PIXEL_BIT_WIDTH; pixelStep++)
  {
    // - - - Apply hardware X flip rule : reverse bit extraction direction 
    u8 bitShift = ((attributes & OBJ_ATTR_X_FLIP_MASK) != 0) ? pixelStep : ((PIXEL_BIT_WIDTH - BIT_MASK_BASE) - pixelStep);
    u8 bitMask  = (BIT_MASK_BASE << bitShift);

    u8 colorBitLow      = (spriteDataLow  & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 colorBitHigh     = (spriteDataHigh & bitMask) ? BIT_MASK_BASE : PIXEL_COLOR_TRANSPARENT;
    u8 finalColorIndex  = (colorBitHigh << PIXEL_SHIFT_HIGH_BIT) | colorBitLow;

    // - - - Rule 1: transparent sprite pixels are completely ignored 
    if (finalColorIndex == PIXEL_COLOR_TRANSPARENT) continue;

    // - - - Target the active queue slot mapped linearly to the screen space 
    u8 targetFifoSlot = (ctx->objFifo.head + pixelStep) % FIFO_CAPACITY;

    // - - - Rule 2: If the quque slot already holds an opaque pixel, it means a higher priority sprite was already injected into this position. We discard the lower priprity pixel 
    if (ctx->objFifo.pixels[targetFifoSlot].colorIndex != PIXEL_COLOR_TRANSPARENT) continue;

    // - - - Rule 3: the slot is valid, package the metadata and overwrite the transparent placeholder 
    PpuPixel spritePixel;
    spritePixel.colorIndex = finalColorIndex;

    // - - - Resolve palette mappings 
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
