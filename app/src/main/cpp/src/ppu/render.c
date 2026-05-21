#include <ppu/ppu.h>
#include <cartridge/cartridge.h>

typedef struct SpriteSample
{
  bool  valid;
  u8    color;
  u8    attributes;
  u8    spriteIndex;
  u8    spriteX;
} SpriteSample;

static u16 getTileDataAddress(const PpuContext* CTX, u8 TILE_ID, u8 TILE_LINE)
{
  if (CTX->registers.lcdc & LCDC_BG_WIN_DATA_MASK)
  {
    return (u16)(TILE_DATA_8000 + (TILE_ID * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
  }

  const i16 signedTile = (i8) TILE_ID;
  return (u16)(TILE_DATA_8800 + (signedTile * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
}

static u8 sampleTileMapColor(const PpuContext* CTX, u16 MAP_BASE, u8 MAP_X, u8 MAP_Y)
{
  const u8 tileX            = (u8)((MAP_X >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u8 tileY            = (u8)((MAP_Y >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u16 mapIndex        = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8 tileId           = CTX->vram[DEFAULT_VRAM_BANK][MAP_BASE + mapIndex];
  const u8 tileLine         = (u8)(MAP_Y & TILE_PIXEL_MASK);
  const u16 tileDataAddress = getTileDataAddress(CTX, tileId, tileLine);
  const u8 dataLow          = CTX->vram[DEFAULT_VRAM_BANK][tileDataAddress];
  const u8 dataHigh         = CTX->vram[DEFAULT_VRAM_BANK][tileDataAddress + 1];
  const u8 bit              = (u8)(TILE_PIXEL_MASK - (MAP_X & TILE_PIXEL_MASK));

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

static u8 getSpriteHeight(u8 LCDC)
{
  return (LCDC & LCDC_OBJ_SIZE_MASK) ? SPRITE_HEIGHT_16 : SPRITE_HEIGHT_8;
}

static bool isSpriteOnScanline(u8 SCANLINE_Y, i16 SPRITE_Y_TOP, u8 SPRITE_HEIGHT)
{
  const i16 spriteYBottom = SPRITE_Y_TOP + SPRITE_HEIGHT;
  return (SCANLINE_Y >= SPRITE_Y_TOP) && (SCANLINE_Y < spriteYBottom);
}

static u8 sampleSpriteColor(
  const PpuContext* CTX,
  u8                TILE_ID,
  u8                ATTRIBUTES,
  u8                SPRITE_HEIGHT,
  u8                LOCAL_X,
  u8                LOCAL_Y)
{
  if (ATTRIBUTES & OBJ_ATTR_Y_FLIP_MASK) LOCAL_Y = (u8)((SPRITE_HEIGHT - 1) - LOCAL_Y);
  if (ATTRIBUTES & OBJ_ATTR_X_FLIP_MASK) LOCAL_X = (u8)(TILE_PIXEL_MASK - LOCAL_X);

  if (SPRITE_HEIGHT == SPRITE_HEIGHT_16)
  {
    TILE_ID &= SPRITE_TILE_MASK_8X16;
    if (LOCAL_Y >= TILE_SIDE)
    {
      TILE_ID += 1;
      LOCAL_Y = (u8)(LOCAL_Y - TILE_SIDE);
    }
  }

  const u8  vramBank    = (ATTRIBUTES & OBJ_ATTR_VRAM_BANK_MASK) ? 1 : DEFAULT_VRAM_BANK;
  const u16 tileAddress = (u16)(TILE_DATA_8000 + (TILE_ID * TILE_BYTES) + (LOCAL_Y * TILE_LINE_BYTES));
  const u8  dataLow     = CTX->vram[vramBank][tileAddress];
  const u8  dataHigh    = CTX->vram[vramBank][tileAddress + 1];
  const u8  bit         = (u8)(TILE_PIXEL_MASK - LOCAL_X);

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

static SpriteSample selectSpriteForPixel(const PpuContext* CTX, u8 X, u8 Y)
{
  SpriteSample best = 
    { 
      .valid        = false, 
      .color        = 0, 
      .attributes   = 0, 
      .spriteIndex  = INVALID_SPRITE_INDEX, 
      .spriteX      = INVALID_SCREEN_X 
    };
  const u8 spriteHeight = getSpriteHeight(CTX->registers.lcdc);
  u8 spritesOnLine      = 0;

  for (u8 spriteIndex = 0; spriteIndex < SPRITE_COUNT; ++spriteIndex)
  {
    const u16 base    = (u16)(spriteIndex * SPRITE_OAM_ENTRY_BYTES);
    const i16 spriteY = (i16)CTX->oam[base + OAM_Y_OFFSET] - SPRITE_Y_OFFSET;
    const i16 spriteX = (i16)CTX->oam[base + OAM_X_OFFSET] - SPRITE_X_OFFSET;

    if (!isSpriteOnScanline(Y, spriteY, spriteHeight)) continue;
    if (spritesOnLine >= SPRITE_MAX_PER_SCANLINE) break;
    spritesOnLine++;

    if (X < spriteX || X >= (spriteX + SPRITE_WIDTH)) continue;

    const u8 attributes = CTX->oam[base + OAM_ATTR_OFFSET];
    const u8 tileId     = CTX->oam[base + OAM_TILE_OFFSET];
    const u8 localX     = (u8)(X - spriteX);
    const u8 localY     = (u8)(Y - spriteY);
    const u8 color      = sampleSpriteColor(CTX, tileId, attributes, spriteHeight, localX, localY);

    if (color == 0) continue;

    const bool betterPriority =
      !best.valid               ||
      (spriteX < best.spriteX)  ||
      ((spriteX == best.spriteX) && (spriteIndex < best.spriteIndex));

    if (!betterPriority) continue;

    best.valid        = true;
    best.color        = color;
    best.attributes   = attributes;
    best.spriteIndex  = spriteIndex;
    best.spriteX      = (u8)spriteX;
  }

  return best;
}

static u16 getWindowTileDataAddress(const PpuContext* CTX, u8 TILE_ID, u8 TILE_LINE)
{
  if (CTX->registers.lcdc & LCDC_BG_WIN_DATA_MASK)
  {
    return (u16)(TILE_DATA_8000 + (TILE_ID * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
  }

  const i16 signedTile = (i8) TILE_ID;
  return (u16)(TILE_DATA_8800 + (signedTile * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
}

static u8 sampleWindowColor(const PpuContext* CTX, u16 MAP_BASE, u8 WINDOW_X, u8 WINDOW_Y)
{
  const u8  tileX           = (u8)((WINDOW_X >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u8  tileY           = (u8)((WINDOW_Y >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u16 mapIndex        = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8  tileId          = CTX->vram[DEFAULT_VRAM_BANK][MAP_BASE + mapIndex];
  const u8  tileLine        = (u8)(WINDOW_Y & TILE_PIXEL_MASK);
  const u16 tileDataAddress = getWindowTileDataAddress(CTX, tileId, tileLine);

  const u8 dataLow  = CTX->vram[DEFAULT_VRAM_BANK][tileDataAddress];
  const u8 dataHigh = CTX->vram[DEFAULT_VRAM_BANK][tileDataAddress + 1];
  const u8 bit      = (u8)(TILE_PIXEL_MASK - (WINDOW_X & TILE_PIXEL_MASK));

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

void ppuRenderFrame(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->currentFrame.palettes.dmg.bgp  = ctx->registers.bgp;
  ctx->currentFrame.palettes.dmg.obp0 = ctx->registers.obp0;
  ctx->currentFrame.palettes.dmg.obp1 = ctx->registers.obp1;

  ppuRenderBgLayer();
  ppuRenderWindowLayer();
  ppuRenderObjLayer();
}

void ppuRenderBgLayer(void)
{
  PpuContext* ctx = ppuGetContext();
  const bool bgEnabled = (ctx->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) != 0;
  const u16  bgMapBase = (ctx->registers.lcdc & LCDC_BG_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;

  for (i32 y = 0; y < HEIGHT; ++y)
  {
    for (i32 x = 0; x < WIDTH; ++x)
    {
      const i32 index       = (y * WIDTH) + x;
      PpuPixel* pixel       = &ctx->currentFrame.pixels[index];
      pixel->bits.layer     = DEFAULT_LAYER_BACKGROUND;
      pixel->bits.paletteId = DEFAULT_BG_PALETTE;
      if (!bgEnabled)
      {
        pixel->bits.colorIndex = EMPTY_PIXEL_COLOR;
        continue;
      }

      const u8 bgX = (u8)(x + ctx->registers.scx);
      const u8 bgY = (u8)(y + ctx->registers.scy);
      pixel->bits.colorIndex = sampleTileMapColor(ctx, bgMapBase, bgX, bgY);
    }
  }
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
      const i32           index   = (y * WIDTH) + x;
      const SpriteSample  sprite  = selectSpriteForPixel(ctx, x, y);
      if (!sprite.valid) continue;

      const bool bgHasVisibleColor = ctx->currentFrame.pixels[index].bits.colorIndex != 0;
      if ((sprite.attributes & OBJ_ATTR_BG_PRIORITY_MASK) && bgHasVisibleColor) continue;

      ctx->currentFrame.pixels[index].bits.layer      = LAYER_OBJECT;
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

void ppuRenderWindowLayer(void)
{
  PpuContext* ctx           = ppuGetContext();
  const bool bgEnabled      = (ctx->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) != 0;
  const bool windowEnabled  = (ctx->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;

  if (!bgEnabled || !windowEnabled) return;
  if (ctx->registers.wx > WINDOW_WX_MAX || ctx->registers.wy >= HEIGHT) return;

  const i16   windowLeft              = (i16)ctx->registers.wx - WINDOW_X_OFFSET;
  const bool  windowHasVisiblePixels  = windowLeft < WIDTH;
  if (!windowHasVisiblePixels) return;

  const u16 windowMapBase = (ctx->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;
  u8        windowLine    = 0;

  for (i32 y = 0; y < HEIGHT; ++y)
  {
    if (y < ctx->registers.wy) continue;

    for (i32 x = 0; x < WIDTH; ++x)
    {
      if (x < windowLeft) continue;

      const u8  windowX = (u8)(x - windowLeft);
      const u8  color   = sampleWindowColor(ctx, windowMapBase, windowX, windowLine);
      const i32 index   = (y * WIDTH) + x;
      ctx->currentFrame.pixels[index].bits.colorIndex = color;
      ctx->currentFrame.pixels[index].bits.layer      = 0;
      ctx->currentFrame.pixels[index].bits.paletteId  = 0;
    }

    windowLine++;
  }
}
