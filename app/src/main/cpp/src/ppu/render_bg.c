#include <ppu/ppu.h>

#define TILE_LINE_BYTES            2
#define PIXEL_COLOR_MASK           0x01
#define EMPTY_PIXEL_COLOR          0
#define DEFAULT_BG_PALETTE         0
#define DEFAULT_LAYER_BACKGROUND   0
#define TILE_PIXEL_MASK            (TILE_SIDE - 1)
#define DEFAULT_VRAM_BANK          0

static u16 getTileDataAddress(const PpuContext* ctx, u8 tileId, u8 tileLine)
{
  if (ctx->registers.lcdc & LCDC_BG_WIN_DATA_MASK)
  {
    return (u16)(TILE_DATA_8000 + (tileId * TILE_BYTES) + (tileLine * TILE_LINE_BYTES));
  }

  const i16 signedTile = (i8) tileId;
  return (u16)(TILE_DATA_8800 + (signedTile * TILE_BYTES) + (tileLine * TILE_LINE_BYTES));
}

static u8 sampleTileMapColor(const PpuContext* ctx, u16 mapBase, u8 mapX, u8 mapY)
{
  const u8 tileX = (u8)((mapX >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u8 tileY = (u8)((mapY >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u16 mapIndex = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8 tileId = ctx->vram[DEFAULT_VRAM_BANK][mapBase + mapIndex];
  const u8 tileLine = (u8)(mapY & TILE_PIXEL_MASK);
  const u16 tileDataAddress = getTileDataAddress(ctx, tileId, tileLine);
  const u8 dataLow = ctx->vram[DEFAULT_VRAM_BANK][tileDataAddress];
  const u8 dataHigh = ctx->vram[DEFAULT_VRAM_BANK][tileDataAddress + 1];
  const u8 bit = (u8)(TILE_PIXEL_MASK - (mapX & TILE_PIXEL_MASK));

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

void ppuRenderBgLayer(void)
{
  PpuContext* ctx = ppuGetContext();
  const bool bgEnabled = (ctx->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) != 0;
  const u16 bgMapBase = (ctx->registers.lcdc & LCDC_BG_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;

  for (i32 y = 0; y < HEIGHT; ++y)
  {
    for (i32 x = 0; x < WIDTH; ++x)
    {
      const i32 index = (y * WIDTH) + x;
      PpuPixel* pixel = &ctx->currentFrame.pixels[index];
      pixel->bits.layer = DEFAULT_LAYER_BACKGROUND;
      pixel->bits.paletteId = DEFAULT_BG_PALETTE;
      pixel->bits.colorIndex = EMPTY_PIXEL_COLOR;

      if (!bgEnabled) continue;

      const u8 bgX = (u8)(x + ctx->registers.scx);
      const u8 bgY = (u8)(y + ctx->registers.scy);
      pixel->bits.colorIndex = sampleTileMapColor(ctx, bgMapBase, bgX, bgY);
    }
  }
}
