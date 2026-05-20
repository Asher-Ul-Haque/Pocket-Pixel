#include <ppu/ppu.h>
#include <cartridge/cartridge.h>

#define TILE_LINE_BYTES          2
#define TILE_PIXEL_MASK          (TILE_SIDE - 1)
#define PIXEL_COLOR_MASK         0x01
#define DMG_OBJ_PALETTE_0        0
#define DMG_OBJ_PALETTE_1        1
#define LAYER_OBJECT             1
#define DEFAULT_VRAM_BANK        0
#define SPRITE_TILE_MASK_8X16    0xFE
#define INVALID_SPRITE_INDEX     0xFF
#define INVALID_SCREEN_X         0xFF

typedef struct SpriteSample
{
  bool valid;
  u8 color;
  u8 attributes;
  u8 spriteIndex;
  u8 spriteX;
} SpriteSample;

static u8 getSpriteHeight(u8 lcdc)
{
  return (lcdc & LCDC_OBJ_SIZE_MASK) ? SPRITE_HEIGHT_16 : SPRITE_HEIGHT_8;
}

static bool isSpriteOnScanline(u8 scanlineY, i16 spriteYTop, u8 spriteHeight)
{
  const i16 spriteYBottom = spriteYTop + spriteHeight;
  return (scanlineY >= spriteYTop) && (scanlineY < spriteYBottom);
}

static u8 sampleSpriteColor(
  const PpuContext* ctx,
  u8 tileId,
  u8 attributes,
  u8 spriteHeight,
  u8 localX,
  u8 localY)
{
  if (attributes & OBJ_ATTR_Y_FLIP_MASK) localY = (u8)((spriteHeight - 1) - localY);
  if (attributes & OBJ_ATTR_X_FLIP_MASK) localX = (u8)(TILE_PIXEL_MASK - localX);

  if (spriteHeight == SPRITE_HEIGHT_16)
  {
    tileId &= SPRITE_TILE_MASK_8X16;
    if (localY >= TILE_SIDE)
    {
      tileId += 1;
      localY = (u8)(localY - TILE_SIDE);
    }
  }

  const u8 vramBank = (attributes & OBJ_ATTR_VRAM_BANK_MASK) ? 1 : DEFAULT_VRAM_BANK;
  const u16 tileAddress = (u16)(TILE_DATA_8000 + (tileId * TILE_BYTES) + (localY * TILE_LINE_BYTES));
  const u8 dataLow = ctx->vram[vramBank][tileAddress];
  const u8 dataHigh = ctx->vram[vramBank][tileAddress + 1];
  const u8 bit = (u8)(TILE_PIXEL_MASK - localX);

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

static SpriteSample selectSpriteForPixel(const PpuContext* ctx, u8 x, u8 y)
{
  SpriteSample best = { .valid = false, .color = 0, .attributes = 0, .spriteIndex = INVALID_SPRITE_INDEX, .spriteX = INVALID_SCREEN_X };
  const u8 spriteHeight = getSpriteHeight(ctx->registers.lcdc);
  u8 spritesOnLine = 0;

  for (u8 spriteIndex = 0; spriteIndex < SPRITE_COUNT; ++spriteIndex)
  {
    const u16 base = (u16)(spriteIndex * SPRITE_OAM_ENTRY_BYTES);
    const i16 spriteY = (i16)ctx->oam[base + OAM_Y_OFFSET] - SPRITE_Y_OFFSET;
    const i16 spriteX = (i16)ctx->oam[base + OAM_X_OFFSET] - SPRITE_X_OFFSET;

    if (!isSpriteOnScanline(y, spriteY, spriteHeight)) continue;

    spritesOnLine++;
    if (spritesOnLine > SPRITE_MAX_PER_SCANLINE) continue;

    if (x < spriteX || x >= (spriteX + SPRITE_WIDTH)) continue;

    const u8 attributes = ctx->oam[base + OAM_ATTR_OFFSET];
    const u8 tileId = ctx->oam[base + OAM_TILE_OFFSET];
    const u8 localX = (u8)(x - spriteX);
    const u8 localY = (u8)(y - spriteY);
    const u8 color = sampleSpriteColor(ctx, tileId, attributes, spriteHeight, localX, localY);

    if (color == 0) continue;

    const bool betterPriority =
      !best.valid ||
      (spriteX < best.spriteX) ||
      ((spriteX == best.spriteX) && (spriteIndex < best.spriteIndex));

    if (!betterPriority) continue;

    best.valid = true;
    best.color = color;
    best.attributes = attributes;
    best.spriteIndex = spriteIndex;
    best.spriteX = (u8)spriteX;
  }

  return best;
}

void ppuRenderObjLayer(void)
{
  PpuContext* ctx = ppuGetContext();
  if ((ctx->registers.lcdc & LCDC_OBJ_ENABLE_MASK) == 0) return;

  const bool isDmg = cartridgeGetContext()->mode == MODE_DMG_GAMEBOY;

  for (u8 y = 0; y < HEIGHT; ++y)
  {
    for (u8 x = 0; x < WIDTH; ++x)
    {
      const i32 index = (y * WIDTH) + x;
      const SpriteSample sprite = selectSpriteForPixel(ctx, x, y);
      if (!sprite.valid) continue;

      const bool bgHasVisibleColor = ctx->currentFrame.pixels[index].bits.colorIndex != 0;
      if ((sprite.attributes & OBJ_ATTR_BG_PRIORITY_MASK) && bgHasVisibleColor) continue;

      ctx->currentFrame.pixels[index].bits.layer = LAYER_OBJECT;
      ctx->currentFrame.pixels[index].bits.colorIndex = sprite.color;

      if (isDmg)
      {
        ctx->currentFrame.pixels[index].bits.paletteId =
          (sprite.attributes & OBJ_ATTR_DMG_PALETTE_MASK) ? DMG_OBJ_PALETTE_1 : DMG_OBJ_PALETTE_0;
      }
      else
      {
        ctx->currentFrame.pixels[index].bits.paletteId = sprite.attributes & OBJ_ATTR_CGB_PALETTE_MASK;
      }
    }
  }
}
