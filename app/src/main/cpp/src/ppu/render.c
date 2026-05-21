#include <ppu/ppu.h>
#include <cartridge/cartridge.h>

typedef struct SpriteSample
{
  bool  valid;
  u8    color;
  u8    attributes;
  u8    spriteIndex;
  i16   spriteX;
} SpriteSample;

typedef struct BgSample
{
  u8    color;
  u8    paletteId;
  bool  priority;
} BgSample;

static bool isCgbHardware(void)
{
  return cartridgeGetContext()->mode != MODE_DMG_GAMEBOY;
}

static u8 resolveDmgShade(const PpuContext* CTX, u8 LAYER, u8 COLOR_INDEX, u8 PALETTE_ID)
{
  const u8 paletteReg = (LAYER == DEFAULT_LAYER_BACKGROUND) ?
    CTX->registers.bgp :
    ((PALETTE_ID == DMG_OBJ_PALETTE_0) ? CTX->registers.obp0 : CTX->registers.obp1);
  return (u8)((paletteReg >> (COLOR_INDEX * 2)) & 0x03);
}

static u16 resolveCgbColor555(const PpuContext* CTX, u8 LAYER, u8 COLOR_INDEX, u8 PALETTE_ID)
{
  const u16* palette = (LAYER == DEFAULT_LAYER_BACKGROUND) ? CTX->currentFrame.palettes.cgb.bg : CTX->currentFrame.palettes.cgb.obj;
  return (u16)(palette[(PALETTE_ID * CGB_PALETTE_COLOR_COUNT) + COLOR_INDEX] & 0x7FFF);
}

static u16 getTileDataAddress(const PpuContext* CTX, u8 TILE_ID, u8 TILE_LINE)
{
  if (CTX->registers.lcdc & LCDC_BG_WIN_DATA_MASK)
  {
    return (u16)(TILE_DATA_8000 + (TILE_ID * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
  }

  const i16 signedTile = (i8) TILE_ID;
  return (u16)(TILE_DATA_8800 + (signedTile * TILE_BYTES) + (TILE_LINE * TILE_LINE_BYTES));
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

static u8 sampleTileColor(const PpuContext* CTX, u8 TILE_ID, u8 TILE_LINE, u8 PIXEL_X, u8 ATTRIBUTES)
{
  u8 localX = PIXEL_X;
  u8 localY = TILE_LINE;

  if (ATTRIBUTES & BG_ATTR_X_FLIP_MASK) localX = (u8)(TILE_PIXEL_MASK - localX);
  if (ATTRIBUTES & BG_ATTR_Y_FLIP_MASK) localY = (u8)(TILE_PIXEL_MASK - localY);

  const u8  vramBank    = ((ATTRIBUTES & BG_ATTR_VRAM_BANK_MASK) && isCgbHardware()) ? 1 : DEFAULT_VRAM_BANK;
  const u16 tileAddress = getTileDataAddress(CTX, TILE_ID, localY);
  const u8  dataLow     = CTX->vram[vramBank][tileAddress];
  const u8  dataHigh    = CTX->vram[vramBank][tileAddress + 1];
  const u8  bit         = (u8)(TILE_PIXEL_MASK - localX);

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

static BgSample sampleBgOrWindow(const PpuContext* CTX, u8 X, u8 Y)
{
  BgSample sample =
  {
    .color     = 0,
    .paletteId = 0,
    .priority  = false
  };

  const bool cgb = isCgbHardware();
  const bool bgMasterEnable = (CTX->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) != 0;
  if (!bgMasterEnable) return sample;

  const bool windowEnabled = (CTX->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;
  const i16  windowLeft    = (i16)CTX->registers.wx - WINDOW_X_OFFSET;
  const bool windowActive  =
    windowEnabled &&
    CTX->registers.wx <= WINDOW_WX_MAX &&
    CTX->registers.wy < HEIGHT &&
    Y >= CTX->registers.wy &&
    windowLeft < WIDTH &&
    X >= windowLeft;

  u8 mapX;
  u8 mapY;
  u16 mapBase;

  if (windowActive)
  {
    mapX = (u8)(X - windowLeft);
    mapY = (u8)(Y - CTX->registers.wy);
    mapBase = (CTX->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;
  }
  else
  {
    mapX = (u8)(X + CTX->registers.scx);
    mapY = (u8)(Y + CTX->registers.scy);
    mapBase = (CTX->registers.lcdc & LCDC_BG_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;
  }

  const u8  tileX    = (u8)((mapX >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u8  tileY    = (u8)((mapY >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u16 mapIndex = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8  tileId   = CTX->vram[DEFAULT_VRAM_BANK][mapBase + mapIndex];
  const u8  attr     = cgb ? CTX->vram[1][mapBase + mapIndex] : 0;
  const u8  tileLine = (u8)(mapY & TILE_PIXEL_MASK);
  const u8  pixelX   = (u8)(mapX & TILE_PIXEL_MASK);

  sample.color     = sampleTileColor(CTX, tileId, tileLine, pixelX, attr);
  sample.paletteId = cgb ? (attr & BG_ATTR_CGB_PALETTE_MASK) : DEFAULT_BG_PALETTE;
  sample.priority  = cgb && ((attr & BG_ATTR_PRIORITY_MASK) != 0);
  return sample;
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

  const u8  vramBank    = (isCgbHardware() && (ATTRIBUTES & OBJ_ATTR_VRAM_BANK_MASK)) ? 1 : DEFAULT_VRAM_BANK;
  const u16 tileAddress = (u16)(TILE_DATA_8000 + (TILE_ID * TILE_BYTES) + (LOCAL_Y * TILE_LINE_BYTES));
  const u8  dataLow     = CTX->vram[vramBank][tileAddress];
  const u8  dataHigh    = CTX->vram[vramBank][tileAddress + 1];
  const u8  bit         = (u8)(TILE_PIXEL_MASK - LOCAL_X);

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

static SpriteSample selectSpriteForPixel(const PpuContext* CTX, u8 X, u8 Y)
{
  const bool dmg = !isCgbHardware();
  SpriteSample best =
  {
    .valid       = false,
    .color       = 0,
    .attributes  = 0,
    .spriteIndex = INVALID_SPRITE_INDEX,
    .spriteX     = 0
  };

  const u8 spriteHeight = getSpriteHeight(CTX->registers.lcdc);
  u8 spritesOnLine      = 0;

  for (u8 spriteIndex = 0; spriteIndex < SPRITE_COUNT; ++spriteIndex)
  {
    const u16 base    = (u16)(spriteIndex * SPRITE_OAM_ENTRY_BYTES);
    const i16 spriteY = (i16)CTX->oam[base + OAM_Y_OFFSET] - SPRITE_Y_OFFSET;
    const i16 spriteX = (i16)CTX->oam[base + OAM_X_OFFSET] - SPRITE_X_OFFSET;
    const i16 xPos    = (i16)X;

    if (!isSpriteOnScanline(Y, spriteY, spriteHeight)) continue;
    if (spritesOnLine >= SPRITE_MAX_PER_SCANLINE) break;
    spritesOnLine++;

    if (xPos < spriteX || xPos >= (spriteX + SPRITE_WIDTH)) continue;

    const u8 attributes = CTX->oam[base + OAM_ATTR_OFFSET];
    const u8 tileId     = CTX->oam[base + OAM_TILE_OFFSET];
    const u8 localX     = (u8)(X - spriteX);
    const u8 localY     = (u8)(Y - spriteY);
    const u8 color      = sampleSpriteColor(CTX, tileId, attributes, spriteHeight, localX, localY);
    if (color == 0) continue;

    if (!best.valid)
    {
      best.valid       = true;
      best.color       = color;
      best.attributes  = attributes;
      best.spriteIndex = spriteIndex;
      best.spriteX     = spriteX;
      continue;
    }

    if (dmg)
    {
      const bool betterPriority =
        (spriteX < best.spriteX) ||
        ((spriteX == best.spriteX) && (spriteIndex < best.spriteIndex));
      if (!betterPriority) continue;
    }
    else
    {
      if (spriteIndex > best.spriteIndex) continue;
    }

    best.color       = color;
    best.attributes  = attributes;
    best.spriteIndex = spriteIndex;
    best.spriteX     = spriteX;
  }

  return best;
}

void ppuRenderScanline(u8 SCANLINE_Y)
{
  if (SCANLINE_Y >= HEIGHT) return;

  PpuContext* ctx = ppuGetContext();
  const bool cgb = isCgbHardware();
  const bool objEnabled = (ctx->registers.lcdc & LCDC_OBJ_ENABLE_MASK) != 0;

  for (u8 x = 0; x < WIDTH; ++x)
  {
    const i32 index = (SCANLINE_Y * WIDTH) + x;
    const BgSample bg = sampleBgOrWindow(ctx, x, SCANLINE_Y);

    u8 finalColor = bg.color;
    u8 finalPalette = bg.paletteId;
    u8 finalLayer = DEFAULT_LAYER_BACKGROUND;

    if (objEnabled)
    {
      const SpriteSample sprite = selectSpriteForPixel(ctx, x, SCANLINE_Y);
      if (sprite.valid)
      {
        const bool bgVisible = bg.color != 0;
        bool objBehindBg = (sprite.attributes & OBJ_ATTR_BG_PRIORITY_MASK) && bgVisible;

        if (cgb && bg.priority && bgVisible)
        {
          objBehindBg = true;
        }

        if (!objBehindBg)
        {
          finalColor = sprite.color;
          finalLayer = LAYER_OBJECT;
          if (cgb) finalPalette = sprite.attributes & OBJ_ATTR_CGB_PALETTE_MASK;
          else     finalPalette = (sprite.attributes & OBJ_ATTR_DMG_PALETTE_MASK) ? DMG_OBJ_PALETTE_1 : DMG_OBJ_PALETTE_0;
        }
      }
    }

    ctx->currentFrame.pixels[index].bits.colorIndex = finalColor;
    ctx->currentFrame.pixels[index].bits.paletteId  = finalPalette;
    ctx->currentFrame.pixels[index].bits.layer      = finalLayer;
    if (cgb) ctx->currentFrame.resolvedColor[index] = resolveCgbColor555(ctx, finalLayer, finalColor, finalPalette);
    else     ctx->currentFrame.resolvedColor[index] = resolveDmgShade(ctx, finalLayer, finalColor, finalPalette);
  }
}

void ppuRenderFrame(void)
{
  PpuContext* ctx = ppuGetContext();
  ctx->currentFrame.palettes.dmg.bgp  = ctx->registers.bgp;
  ctx->currentFrame.palettes.dmg.obp0 = ctx->registers.obp0;
  ctx->currentFrame.palettes.dmg.obp1 = ctx->registers.obp1;
}
