#include "../../include/ppu.h"
#include "../../../GameBoyCore.h"
#include "../../include/interrupt.h"
#include "../../include/common.h"


// Static PPU context
static PPUContext ctx;
PPUContext* ppuGetContext() { return &ctx; }

// - - - Helper Functions - - -
static inline u8 ppuGetColorIdBits(u8 COLOR_BIT, u8 LO, u8 HI) {
  u8 hi = (HI >> COLOR_BIT) & 0x1;
  u8 lo = (LO >> COLOR_BIT) & 0x1;
  return (hi << 1) | lo;
}

static inline bool ppuIsWindowVisibleForLine() {
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 5) && (ctx->wy <= ctx->ly);
}

static inline u8 ppuGetSpriteSize() {
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 2) ? 16 : 8;
}

static inline bool ppuIsXFlipped(u8 ATTR) { return BIT(ATTR, 5); }
static inline bool ppuIsYFlipped(u8 ATTR) { return BIT(ATTR, 6); }
static inline bool ppuIsTransparent(u8 COLOR_ID) { return COLOR_ID == 0; }
static inline bool ppuIsAboveBG(u8 ATTR) { return !BIT(ATTR, 7); }

static inline bool ppuIsBGTransparent(i32 x, i32 y) {
  PPUContext* ctx = ppuGetContext();
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return true;
  return ctx->frameBufferBack[x + (y * SCREEN_WIDTH)] == ctx->backgroundPalette[0];
}

static inline u16 ppuGetBackgroundTileMapAddress() {
  return BIT(ctx.lcdc, 3) ? 0x9C00 : 0x9800;
}

static inline u16 ppuGetWindowTileMapAddress() {
  return BIT(ctx.lcdc, 6) ? 0x9C00 : 0x9800;
}

static inline u16 ppuGetTileDataAddress() {
  return BIT(ctx.lcdc, 4) ? 0x8000 : 0x8800;
}

static inline bool ppuIsSignedAddress() {
  return !BIT(ctx.lcdc, 4);
}

// - - - Initialization - - -
void ppuInit() {
  memset(&ctx, 0, sizeof(PPUContext));

  ctx.lcdc = 0x91;
  ctx.stat = 0x00;
  ctx.scy = 0x00;
  ctx.scx = 0x00;
  ctx.ly = 0x00;
  ctx.lyc = 0x00;
  ctx.bgp = 0xFC;
  ctx.obp0 = 0xFF;
  ctx.obp1 = 0xFF;
  ctx.wy = 0x00;
  ctx.wx = 0x00;

  ctx.isEnabled = BIT(ctx.lcdc, 7);
  ctx.scanlineCounter = 0;
  ctx.windowInternalLine = 0;

  // Init palettes
  ppuCachePalette(ctx.backgroundPalette, getColorScheme(), ctx.bgp);
  ppuCachePalette(ctx.objectPalette0, getColorScheme(), ctx.obp0);
  ppuCachePalette(ctx.objectPalette1, getColorScheme(), ctx.obp1);

  // Fill both buffers with background color 0
  u32 bgColor = getColorScheme()[0];
  for (i32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
    ctx.frameBufferFront[i] = bgColor;
    ctx.frameBufferBack[i] = bgColor;
  }

  ctx.frameBuffer = ctx.frameBufferFront;

  for (i32 i = 0; i < 41; ++i)
    ctx.orderBuffer[i] = -1;

  ppuUpdateStatMode(MODE_OAM);
}

// - - - Coincidence Flag - - -
void ppuHandleCoincidenceFlag() {
  if (ctx.ly == ctx.lyc) {
    BIT_SET(ctx.stat, 2, 1);
    if (BIT(ctx.stat, 6)) cpuRequestInterrupt(IT_LCD_STAT);
  } else {
    BIT_SET(ctx.stat, 2, 0);
  }
}

// - - - STAT Mode Update - - -
void ppuUpdateStatMode(PPUMode MODE) {
  BIT_SET(ctx.stat, 0, 0);
  BIT_SET(ctx.stat, 1, 0);
  ctx.stat |= MODE;
}

// - - - PPU Tick - - -
void ppuTick() {
  if (!ctx.isEnabled) return;

  ctx.scanlineCounter++;

  PPUMode currentMode = (PPUMode)(ctx.stat & 0x3);

  switch (currentMode) {
    case MODE_OAM:
      if (ctx.scanlineCounter >= OAM_CYCLES) {
        ctx.scanlineCounter -= OAM_CYCLES;
        ppuUpdateStatMode(MODE_VRAM);
        if (BIT(ctx.stat, 5)) cpuRequestInterrupt(IT_LCD_STAT);
      }
      break;

    case MODE_VRAM:
      if (ctx.scanlineCounter >= VRAM_CYCLES) {
        ctx.scanlineCounter -= VRAM_CYCLES;

        // Latch scroll values here for this scanline
        ctx.scrollX_latched = ctx.scx;
        ctx.scrollY_latched = ctx.scy;

        ppuUpdateStatMode(MODE_HBLANK);
        ppuDrawScanLine();
      }
      break;

    case MODE_HBLANK:
      if (ctx.scanlineCounter >= HBLANK_CYCLES) {
        ctx.scanlineCounter -= HBLANK_CYCLES;
        ctx.ly++;
        ppuHandleCoincidenceFlag();

        if (BIT(ctx.stat, 3)) cpuRequestInterrupt(IT_LCD_STAT);

        if (ctx.ly == SCREEN_HEIGHT) {
          ppuUpdateStatMode(MODE_VBLANK);
          cpuRequestInterrupt(IT_VBLANK);
          if (BIT(ctx.stat, 4)) cpuRequestInterrupt(IT_LCD_STAT);

          // Swap front/back buffers
// Swap contents of back buffer into front buffer
          memcpy(ctx.frameBufferFront, ctx.frameBufferBack, sizeof(ctx.frameBufferFront));
          ctx.frameBuffer = ctx.frameBufferFront;


          render();
        } else {
          ppuUpdateStatMode(MODE_OAM);
        }
      }
      break;

    case MODE_VBLANK:
      if (ctx.scanlineCounter >= SCANLINE_CYCLES) {
        ctx.scanlineCounter -= SCANLINE_CYCLES;
        ctx.ly++;

        ppuHandleCoincidenceFlag();

        if (BIT(ctx.stat, 4)) cpuRequestInterrupt(IT_LCD_STAT);

        if (ctx.ly > SCREEN_VBLANK_HEIGHT) {
          ppuUpdateStatMode(MODE_OAM);
          ctx.ly = 0;
          ctx.windowInternalLine = 0;
          ppuHandleCoincidenceFlag();
        }
      }
      break;
  }
}

// - - - Scanline Drawing - - -
void ppuDrawScanLine() {
  ppuRenderBG();
  if (BIT(ctx.lcdc, 1)) ppuRenderSpritesBuffer();
}

// - - - BG Rendering - - -
void ppuRenderBG() {
  i32 wxStart = ctx.wx - 7;
  u8 ly = ctx.ly;

  bool isWindowVisible = ppuIsWindowVisibleForLine();
  bool windowTriggered = false;

  if (ly == ctx.wy && isWindowVisible)
    ctx.windowInternalLine = 0;

  u16 windowTileMap = ppuGetWindowTileMapAddress();
  u16 bgTileMap = ppuGetBackgroundTileMapAddress();
  u16 tileDataBase = ppuGetTileDataAddress();
  bool signedAddress = ppuIsSignedAddress();

  u8 lo = 0, hi = 0;

  for (i32 p = 0; p < SCREEN_WIDTH; ++p) {
    bool inWindow = isWindowVisible && (p >= wxStart);
    if (inWindow) windowTriggered = true;

    u8 effectiveX = inWindow ? (p - wxStart) : (p + ctx.scrollX_latched);
    u8 effectiveY = inWindow ? ctx.windowInternalLine : (ly + ctx.scrollY_latched);

    if (p == 0 || (effectiveX & 0x7) == 0) {
      u8 tileCol = effectiveX / 8;
      u8 tileRow = effectiveY / 8;
      u16 mapAddr = inWindow ? windowTileMap : bgTileMap;
      u16 tileAddr = mapAddr + (tileRow * 32) + tileCol;
      u8 tileNum = ppuVRAMread(tileAddr);

      if (signedAddress)
        tileNum = (u8)((i8)tileNum + 128);

      u8 tileLine = (effectiveY & 0x7) * 2;
      u16 tileLoc = tileDataBase + (tileNum * 16);

      lo = ppuVRAMread(tileLoc + tileLine);
      hi = ppuVRAMread(tileLoc + tileLine + 1);
    }

    u8 bit = 7 - (effectiveX & 0x7);
    u8 colorId = ppuGetColorIdBits(bit, lo, hi);
    u32 color = ctx.backgroundPalette[colorId];
    ctx.frameBufferBack[p + (ly * SCREEN_WIDTH)] = color;
  }

  if (windowTriggered)
    ctx.windowInternalLine++;
}

// - - - Sprite Drawing - - -
static void ppuOrderSprites(u8 currentLY, u8 OBJ_SIZE, i32* ORDER_BUFFER) {
  i32 count = 0;
  for (i32 i = 0; i < 40; ++i) {
    u8 y = ppuOAMread(i * 4);
    if ((currentLY + 16) >= y && (currentLY + 16) < (y + OBJ_SIZE)) {
      if (count < 10)
        ORDER_BUFFER[count++] = i * 4;
    }
  }
  ORDER_BUFFER[count] = -1;

  for (i32 i = 0; i < count - 1; ++i) {
    for (i32 j = 0; j < count - i - 1; ++j) {
      u8 x1 = ppuOAMread(ORDER_BUFFER[j] + 1);
      u8 x2 = ppuOAMread(ORDER_BUFFER[j + 1] + 1);
      if (x1 > x2 || (x1 == x2 && ORDER_BUFFER[j] > ORDER_BUFFER[j + 1])) {
        i32 temp = ORDER_BUFFER[j];
        ORDER_BUFFER[j] = ORDER_BUFFER[j + 1];
        ORDER_BUFFER[j + 1] = temp;
      }
    }
  }

  if (count > 10)
    ORDER_BUFFER[10] = -1;
}

void ppuRenderSpritesBuffer() {
  u8 ly = ctx.ly;
  u8 size = ppuGetSpriteSize();
  ppuOrderSprites(ly, size, ctx.orderBuffer);

  i32 index = 0;
  while (ctx.orderBuffer[index] != -1) {
    i32 i = ctx.orderBuffer[index];
    u8 y = ppuOAMread(i + 0);
    u8 x = ppuOAMread(i + 1);
    u8 tile = ppuOAMread(i + 2);
    u8 attr = ppuOAMread(i + 3);

    i32 sx = x - 8;
    i32 sy = y - 16;

    if (sx >= SCREEN_WIDTH || sx + 8 <= 0) {
      index++;
      continue;
    }

    u8 actualTile = tile;
    i32 row = ly - sy;

    if (ppuIsYFlipped(attr)) row = (size - 1) - row;

    if (size == 16) {
      if (row >= 8) actualTile = tile | 0x01;
      else actualTile = tile & 0xFE;
      row %= 8;
    }

    u16 dataAddr = 0x8000 + (actualTile * 16) + (row * 2);
    u8 lo = ppuVRAMread(dataAddr);
    u8 hi = ppuVRAMread(dataAddr + 1);
    u32* palette = BIT(attr, 4) ? ctx.objectPalette1 : ctx.objectPalette0;

    for (i32 p = 0; p < 8; ++p) {
      i32 px = sx + p;
      if (px < 0 || px >= SCREEN_WIDTH) continue;

      u8 bit = ppuIsXFlipped(attr) ? p : (7 - p);
      u8 colorId = ppuGetColorIdBits(bit, lo, hi);
      if (ppuIsTransparent(colorId)) continue;

      if (ppuIsAboveBG(attr) || ppuIsBGTransparent(px, ly))
        ctx.frameBufferBack[px + (ly * SCREEN_WIDTH)] = palette[colorId];
    }

    index++;
  }
}
