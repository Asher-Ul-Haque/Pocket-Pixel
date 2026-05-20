#include <ppu/ppu.h>

#define TILE_LINE_BYTES          2
#define PIXEL_COLOR_MASK         0x01
#define TILE_PIXEL_MASK          (TILE_SIDE - 1)
#define DEFAULT_VRAM_BANK        0

static u16 getWindowTileDataAddress(const PpuContext* ctx, u8 tileId, u8 tileLine)
{
  if (ctx->registers.lcdc & LCDC_BG_WIN_DATA_MASK)
  {
    return (u16)(TILE_DATA_8000 + (tileId * TILE_BYTES) + (tileLine * TILE_LINE_BYTES));
  }

  const i16 signedTile = (i8) tileId;
  return (u16)(TILE_DATA_8800 + (signedTile * TILE_BYTES) + (tileLine * TILE_LINE_BYTES));
}

static u8 sampleWindowColor(const PpuContext* ctx, u16 mapBase, u8 windowX, u8 windowY)
{
  const u8 tileX = (u8)((windowX >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u8 tileY = (u8)((windowY >> TILE_ROW_SHIFT) & TILE_MAP_MASK);
  const u16 mapIndex = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8 tileId = ctx->vram[DEFAULT_VRAM_BANK][mapBase + mapIndex];
  const u8 tileLine = (u8)(windowY & TILE_PIXEL_MASK);
  const u16 tileDataAddress = getWindowTileDataAddress(ctx, tileId, tileLine);

  const u8 dataLow = ctx->vram[DEFAULT_VRAM_BANK][tileDataAddress];
  const u8 dataHigh = ctx->vram[DEFAULT_VRAM_BANK][tileDataAddress + 1];
  const u8 bit = (u8)(TILE_PIXEL_MASK - (windowX & TILE_PIXEL_MASK));

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

void ppuRenderWindowLayer(void)
{
  PpuContext* ctx = ppuGetContext();
  const bool bgEnabled = (ctx->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) != 0;
  const bool windowEnabled = (ctx->registers.lcdc & LCDC_WIN_ENABLE_MASK) != 0;

  if (!bgEnabled || !windowEnabled) return;
  if (ctx->registers.wx > WINDOW_X_MAX || ctx->registers.wy >= HEIGHT) return;

  const i16 windowLeft = (i16)ctx->registers.wx - WINDOW_X_OFFSET;
  const u16 windowMapBase = (ctx->registers.lcdc & LCDC_WIN_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;

  for (i32 y = ctx->registers.wy; y < HEIGHT; ++y)
  {
    for (i32 x = 0; x < WIDTH; ++x)
    {
      if (x < windowLeft) continue;

      const u8 windowX = (u8)(x - windowLeft);
      const u8 windowY = (u8)(y - ctx->registers.wy);
      const u8 color = sampleWindowColor(ctx, windowMapBase, windowX, windowY);
      const i32 index = (y * WIDTH) + x;
      ctx->currentFrame.pixels[index].bits.colorIndex = color;
      ctx->currentFrame.pixels[index].bits.layer = 0;
      ctx->currentFrame.pixels[index].bits.paletteId = 0;
    }
  }
}
