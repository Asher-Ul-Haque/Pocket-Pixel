#include <ppu/ppu.h>
#include <cartridge/cartridge.h>

#define TILE_LINE_BYTES          2
#define PIXELS_PER_TILE          TILE_SIDE
#define PIXEL_COLOR_MASK         0x01
#define EMPTY_PIXEL_COLOR        0
#define DEFAULT_BG_PALETTE       0
#define DEFAULT_LAYER_BACKGROUND 0

static u16 getBgMapBase(const PpuContext* CTX)
{
  return (CTX->registers.lcdc & LCDC_BG_TILE_MAP_MASK) ? BG_MAP_1_OFFSET : BG_MAP_0_OFFSET;
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

static u8 sampleBgColor(const PpuContext* CTX, i32 X, i32 Y)
{
  const u16 mapBase = getBgMapBase(CTX);
  const u8 bgX = (u8)(X + CTX->registers.scx);
  const u8 bgY = (u8)(Y + CTX->registers.scy);
  const u8 tileX = (u8)((bgX >> 3) & TILE_MAP_MASK);
  const u8 tileY = (u8)((bgY >> 3) & TILE_MAP_MASK);
  const u16 mapIndex = (u16)(tileY * TILE_MAP_WIDTH + tileX);
  const u8 tileId = CTX->vram[0][mapBase + mapIndex];
  const u8 tileLine = (u8)(bgY & (PIXELS_PER_TILE - 1));
  const u16 tileDataAddress = getTileDataAddress(CTX, tileId, tileLine);

  const u8 dataLow = CTX->vram[0][tileDataAddress];
  const u8 dataHigh = CTX->vram[0][tileDataAddress + 1];
  const u8 bit = (u8)(7 - (bgX & (PIXELS_PER_TILE - 1)));

  return (u8)((((dataHigh >> bit) & PIXEL_COLOR_MASK) << 1) | ((dataLow >> bit) & PIXEL_COLOR_MASK));
}

void ppuRenderFrame(void)
{
  PpuContext* ctx = ppuGetContext();

  ctx->currentFrame.palettes.dmg.bgp  = ctx->registers.bgp;
  ctx->currentFrame.palettes.dmg.obp0 = ctx->registers.obp0;
  ctx->currentFrame.palettes.dmg.obp1 = ctx->registers.obp1;

  for (i32 y = 0; y < HEIGHT; ++y)
  {
    for (i32 x = 0; x < WIDTH; ++x)
    {
      const i32 index = (y * WIDTH) + x;
      PpuPixel* pixel = &ctx->currentFrame.pixels[index];

      pixel->bits.layer = DEFAULT_LAYER_BACKGROUND;
      pixel->bits.paletteId = DEFAULT_BG_PALETTE;

      if ((ctx->registers.lcdc & LCDC_BG_WIN_ENABLE_MASK) == 0)
      {
        pixel->bits.colorIndex = EMPTY_PIXEL_COLOR;
        continue;
      }

      pixel->bits.colorIndex = sampleBgColor(ctx, x, y);
    }
  }
}
