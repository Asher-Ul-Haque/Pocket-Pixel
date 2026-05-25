#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cartridge/cartridge.h>


void ppuExecuteOamScan(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->scanlineOamBuffer.spriteCount = OAM_DMA_START_INDEX;

  u8 activeSpriteHeight = SPRITE_HEIGHT_8;
  if ((ctx->registers.lcdc & LCDC_OBJ_SIZE_MASK) != 0)
  { activeSpriteHeight = SPRITE_HEIGHT_16; }

  // - - - Sacn all 40 entires seuqneitally from the OAM array 
  for (u8 spriteIndex = OAM_DMA_START_INDEX; spriteIndex < SPRITE_COUNT; ++spriteIndex)
  {
    u16 oamBaseOffset = (u16) spriteIndex * SPRITE_OAM_ENTRY_BYTES;
    u8  spriteY       = ctx->oam[oamBaseOffset + OAM_Y_OFFSET];

    if ((ctx->registers.ly + SPRITE_Y_OFFSET >= spriteY) &&
        (ctx->registers.ly + SPRITE_Y_OFFSET <  spriteY + activeSpriteHeight))
    {
      if (ctx->scanlineOamBuffer.spriteCount < SPRITE_MAX_PER_SCANLINE)
      {
        ctx->scanlineOamBuffer.spriteIndices[ctx->scanlineOamBuffer.spriteCount] = spriteIndex;
        ctx->scanlineOamBuffer.spriteCount++;
      }
      else break;
    }
  }

  // - - - DMG Horizontal priority sorting override 
  if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY && ctx->scanlineOamBuffer.spriteCount > PIXEL_COLOR_MASK)
  {
    // - - - insertion sort 
    for (u8 cursor = PIXEL_COLOR_MASK; cursor < ctx->scanlineOamBuffer.spriteCount; ++cursor)
    {
      u8  currentSpriteIndex  = ctx->scanlineOamBuffer.spriteIndices[cursor];
      u16 currentOamBase      = (u16) currentSpriteIndex * SPRITE_OAM_ENTRY_BYTES;
      u8  currentX            = ctx->oam[currentOamBase + OAM_X_OFFSET];

      i32 shiftIndex = (i32) cursor - PIXEL_COLOR_MASK;
      while (shiftIndex >= OAM_DMA_START_INDEX)
      {
        u8  compareSpriteIndex  = ctx->scanlineOamBuffer.spriteIndices[shiftIndex];
        u16 compareOamBase      = (u16) compareSpriteIndex * SPRITE_OAM_ENTRY_BYTES;
        u8  compareX            = ctx->oam[compareOamBase + OAM_X_OFFSET];

        if (compareX > currentX)
        {
          ctx->scanlineOamBuffer.spriteIndices[shiftIndex + PIXEL_COLOR_MASK] = compareSpriteIndex;
          shiftIndex--;
        }
        else break;
      }
      ctx->scanlineOamBuffer.spriteIndices[shiftIndex + PIXEL_COLOR_MASK] = currentSpriteIndex;
    }
  }
}

u16 ppuGetSpriteTimingPenalties(void)
{
  PpuContext* ctx             = ppuGetContext();
  u16         totalStallDots  = OAM_DMA_START_INDEX;

  // - - - if sprites are completely disbaled in LCDC, they incur zero timing 
  if ((ctx->registers.lcdc & LCDC_OBJ_ENABLE_MASK) == 0) return totalStallDots;

  // - - - Process each selected sprite currently sitting inside the active line buffer 
  for (u8 scanIndex = OAM_DMA_START_INDEX; scanIndex < ctx->scanlineOamBuffer.spriteCount; ++scanIndex)
  {
    u8  targetSpriteTableIndex  = ctx->scanlineOamBuffer.spriteIndices[scanIndex];
    u16 oamAddressBase          = (u16) targetSpriteTableIndex * SPRITE_OAM_ENTRY_BYTES;
    u8  spriteX                 = ctx->oam[oamAddressBase + OAM_X_OFFSET];

    // - - - Sprite X = 0 bypasses fine scroll checks and inflicts a flat 11-dot hardware penalty 
    if (spriteX == OAM_DMA_START_INDEX)
    {
      totalStallDots += SPRITE_X_0_PENALTY;
      continue;
    }

    // - - - standard elastic formula : 6 base dots for the physical VRAM data fetchplus an additional shift penalty based on tile alignment offsets. 
    u8 scrollOffsetShift = (ctx->registers.scx + spriteX) & TILE_PIXEL_MASK;
    totalStallDots += VRAM_DATA_FETCH + scrollOffsetShift;
  }

  return totalStallDots;
}
