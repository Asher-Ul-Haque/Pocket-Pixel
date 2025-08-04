#include "../../include/ppu.h"
#include "../../../GameBoyCore.h" 
#include "../../include/interrupt.h"
#include "../../include/common.h"

static PPUContext   ctx;
PPUContext* ppuGetContext           ()          { return &ctx; }



// - - - Helper Functions - - -

// - - - Helper to get color ID bits from lo and hi tile data bytes
static inline u8 ppuGetColorIdBits(u8 COLOR_BIT, u8 LO, u8 HI) 
{
  u8 hi = (HI >> COLOR_BIT) & 0x1;
  u8 lo = (LO >> COLOR_BIT) & 0x1;
  return (hi << 1) | lo;
}

// - - - Check if window is visible on the current line based on LCDC, WY, LY
static inline bool ppuIsWindowVisibleForLine() 
{
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 5) && (ctx->wy <= ctx->ly);
}

// - - - Get sprite size (8x8 or 8x16) based on LCDC bit 2
static inline u8 ppuGetSpriteSize() 
{
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 2) ? 16 : 8;
}

static inline bool ppuIsXFlipped    (u8 ATTR)       { return BIT(ATTR, 5); }
static inline bool ppuIsYFlipped    (u8 ATTR)       { return BIT(ATTR, 6); }
static inline bool ppuIsTransparent (u8 COLOR_ID)   { return COLOR_ID == 0; }
static inline bool ppuIsAboveBG     (u8 ATTR)       { return !BIT(ATTR, 7); }

// - - - Check if background pixel at (x, y) is color 0 (white)
static inline bool ppuIsBGTransparent(i32 x, i32 y) 
{
  PPUContext* ctx = ppuGetContext();
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)  return true;
  else                                                            return ctx->frameBuffer[x + (y * SCREEN_WIDTH)] == ctx->backgroundPalette[0];
}

// - - - Get background tile map address based on LCDC bit 3
static inline u16 ppuGetBackgroundTileMapAddress() 
{
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 3) ? 0x9C00 : 0x9800;
}

// - - - Get window tile map address based on LCDC bit 6
static inline u16 ppuGetWindowTileMapAddress() 
{
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 6) ? 0x9C00 : 0x9800;
}

// - - - Get tile data address based on LCDC bit 4
static inline u16 ppuGetTileDataAddress() 
{
  PPUContext* ctx = ppuGetContext();
  return BIT(ctx->lcdc, 4) ? 0x8000 : 0x8800;
}

// - - - Check if tile data addressing is signed (LCDC bit 4)
static inline bool ppuIsSignedAddress() 
{
  PPUContext* ctx = ppuGetContext();
  return !BIT(ctx->lcdc, 4); 
}


// - - - PPU Implementation - - -

void ppuInit() 
{
  PPUContext* ctx = ppuGetContext();

  memset(ctx,  0, sizeof(PPUContext));  

  // - - - Initialize registers to typical power-up values 
  ctx->lcdc = 0x91;
  ctx->stat = 0x00; 
  ctx->scy  = 0x00;
  ctx->scx  = 0x00;
  ctx->ly   = 0x00;
  ctx->lyc  = 0x00;
  ctx->bgp  = 0xFC;
  ctx->obp0 = 0xFF;
  ctx->obp1 = 0xFF;
  ctx->wy   = 0x00;
  ctx->wx   = 0x00;

  ctx->isEnabled = BIT(ctx->lcdc, 7);
    
  // - - - Initialize internal counter
  ctx->scanlineCounter      = 0;
  ctx->windowInternalLine   = 0;

  // - - - Fill framebuffer with background color 0 (white)
  for (i32 i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) 
  { ctx->frameBuffer[i] = getColorScheme()[0]; }

  // - - - Cache initial palettes
  ppuCachePalette(ctx->backgroundPalette,   getColorScheme(), ctx->bgp);
  ppuCachePalette(ctx->objectPalette0,      getColorScheme(), ctx->obp0);
  ppuCachePalette(ctx->objectPalette1,      getColorScheme(), ctx->obp1);

  // - - - Initialize sprite order buffer with terminator
  for (i32 i = 0; i < 40 + 1; ++i) { ctx->orderBuffer[i] = -1; }
  ppuUpdateStatMode(MODE_OAM);
}


// - - - LY == compares
void ppuHandleCoincidenceFlag() 
{
  PPUContext* ctx = ppuGetContext();
  if (ctx->ly == ctx->lyc) 
  {
    BIT_SET(ctx->stat, 2, 1); 
    if (BIT(ctx->stat, 6))   cpuRequestInterrupt(IT_LCD_STAT); 
  } 
  else BIT_SET(ctx->stat, 2, 0); 
}

void ppuUpdateStatMode(PPUMode MODE) 
{
  PPUContext* ctx = ppuGetContext();
  BIT_SET(ctx->stat, 0, 0); 
  BIT_SET(ctx->stat, 1, 0); 
  ctx->stat |= MODE;
}

void ppuTick() 
{
  PPUContext* ctx = ppuGetContext();

  if (!ctx->isEnabled) return; 
  static int index = 0;


  ctx->scanlineCounter ++;

  PPUMode currentMode = (PPUMode)(ctx->stat & 0x3);

  switch (currentMode) 
  { 
    case MODE_OAM: 
      if (ctx->scanlineCounter >= OAM_CYCLES) 
      {
        ctx->scanlineCounter -= OAM_CYCLES;
        ppuUpdateStatMode(MODE_VRAM);  
        if (BIT(ctx->stat, 5)) cpuRequestInterrupt(IT_LCD_STAT);
      }
      break;

    case MODE_VRAM: 
      if (ctx->scanlineCounter >= VRAM_CYCLES) 
      { 
        ctx->scanlineCounter -= VRAM_CYCLES;
        ppuUpdateStatMode(MODE_HBLANK); 
        ppuDrawScanLine(); 
      }
      break;

    case MODE_HBLANK: 
      if (ctx->scanlineCounter >= HBLANK_CYCLES) 
      {
        ctx->scanlineCounter -= HBLANK_CYCLES;
        ctx->ly++; 
        ppuHandleCoincidenceFlag(); 

        if (BIT(ctx->stat, 3)) cpuRequestInterrupt(IT_LCD_STAT);

        if (ctx->ly == SCREEN_HEIGHT) 
        { 
          ppuUpdateStatMode(MODE_VBLANK);
          cpuRequestInterrupt(IT_VBLANK);
          if (BIT(ctx->stat, 4)) cpuRequestInterrupt(IT_LCD_STAT);
          render();
        }
        else ppuUpdateStatMode(MODE_OAM);
      }
      break;

    case MODE_VBLANK:
      if (ctx->scanlineCounter >= SCANLINE_CYCLES) 
      {
        ctx->scanlineCounter -= SCANLINE_CYCLES;
        ctx->ly++; 

        ppuHandleCoincidenceFlag(); 

        if (BIT(ctx->stat, 4)) { cpuRequestInterrupt(IT_LCD_STAT); }

        if (ctx->ly > SCREEN_VBLANK_HEIGHT) 
        { 
          ppuUpdateStatMode(MODE_OAM);
          ctx->ly                 = 0;
          ctx->windowInternalLine = 0;
          ppuHandleCoincidenceFlag(); 
        }
      }
      break;
    }
}

// - - - Scanline Drawing
void ppuDrawScanLine() 
{
  ppuRenderBG();
  if (BIT(ppuGetContext()->lcdc, 1)) ppuRenderSpritesBuffer();
}

// - - - Background and Window Rendering
void ppuRenderBG() 
{
  PPUContext* ctx = ppuGetContext();

  i32 wxStartOnScreen = ctx->wx - 7;
  u8  windowY         = ctx->wy;
  u8  ly              = ctx->ly;
  u8  screenY         = ctx->scy;
  u8  screenX         = ctx->scx;

  bool isWindowActiveForLine    = ppuIsWindowVisibleForLine();
  bool windowAppearedOnThisLine = false;
  if (ly == windowY && isWindowActiveForLine) ctx->windowInternalLine = 0;

  u16  windowTileMapAddress = ppuGetWindowTileMapAddress();
  u16  bgTileMapAddress     = ppuGetBackgroundTileMapAddress();
  u16  tileDataAddressBase  = ppuGetTileDataAddress();
  bool useSignedAddressing  = ppuIsSignedAddress();

  u8 currentLo = 0;
  u8 currentHi = 0;

  // - - - iterate through ecah pixel on the scanline AND CHECK IF THE CURRENT PIXEL IS within bonds
  for (i32 p = 0; p < SCREEN_WIDTH; ++p) 
  { 
    bool inWindow = isWindowActiveForLine && (p >= wxStartOnScreen);
    if (inWindow) windowAppearedOnThisLine = true;

    u8 effectiveX = inWindow ? (p - wxStartOnScreen) : (p + screenX);
    u8 effectiveY = inWindow ? ctx->windowInternalLine : (ly + screenY);

    if (p == 0 || (effectiveX & 0x7) == 0) 
    {
      u8  tileCol            = effectiveX / 8;
      u8  tileRow            = effectiveY / 8;
      u16 tileMap            = inWindow ? windowTileMapAddress : bgTileMapAddress;
      u16 tileAddressInMap   = tileMap + (tileRow * 32) + tileCol;
      u8  tileNumber         = ppuVRAMread(tileAddressInMap);

      if (useSignedAddressing) tileNumber = (u8)((i8)tileNumber + 128);

      u8  tileLine = (effectiveY & 0x7) * 2;
      u16 tileLoc  = tileDataAddressBase + (tileNumber * 16);

      currentLo = ppuVRAMread(tileLoc + tileLine);
      currentHi = ppuVRAMread(tileLoc + tileLine + 1);
    }

    // - - - Get the color ID for the current pixel
    u8  colorBit = 7 - (effectiveX & 0x7); 
    u8  colorId  = ppuGetColorIdBits(colorBit, currentLo, currentHi);
    u32 color    = ctx->backgroundPalette[colorId];

    ctx->frameBuffer[p + (ly * SCREEN_WIDTH)] = color;
  }

  // - - - Increment windowInternalLine only if the window was active on this scanline
  if (windowAppearedOnThisLine) ctx->windowInternalLine++;
}

// - - - Sprite Ordering Helper 
static void ppuOrderSprites(u8 currentLY, u8 OBJ_SIZE, i32* ORDER_BUFFER) 
{
  PPUContext* ctx   = ppuGetContext();
  i32         count = 0;

  // - - - First, filter sprites by Y-range and collect their OAM addresses
  for (i32 i = 0; i < 40; ++i) 
  {
    u8 spriteY = ppuOAMread(i * 4); 
    if ((currentLY + 16) >= spriteY && (currentLY + 16) < (spriteY + OBJ_SIZE)) 
    {
      if (count < 10) ORDER_BUFFER[count++] = i * 4; 
    }
  }
  ORDER_BUFFER[count] = -1; 

  // - - - Sort the visible sprites by X-coordinate (ascending)
  for (i32 i = 0; i < count - 1; ++i) 
  {
    for (i32 j = 0; j < count - i - 1; ++j) 
    {
      u8 x1 = ppuOAMread(ORDER_BUFFER[j] + 1);
      u8 x2 = ppuOAMread(ORDER_BUFFER[j+1] + 1);        
      if (x1 > x2 || (x1 == x2 && ORDER_BUFFER[j] > ORDER_BUFFER[j+1])) 
      {
        // - - - Swap if x1 is greater, or if x1 == x2 and current sprite has higher OAM index
        i32 temp            = ORDER_BUFFER[j];
        ORDER_BUFFER[j]     = ORDER_BUFFER[j+1];
        ORDER_BUFFER[j+1]   = temp;
      }
    }
  }

  // - - - Ensure only up to 10 sprites are processed (though the filtering already limits this)
  if (count > 10) ORDER_BUFFER[10] = -1;
}

void ppuRenderSpritesBuffer() 
{
  PPUContext* ctx        = ppuGetContext();
  u8          LY         = ctx->ly;
  u8          spriteSize = ppuGetSpriteSize();

  // - - - Populate and sort the sprite order buffer for the current scanline
  ppuOrderSprites(LY, spriteSize, ctx->orderBuffer);

  // - - - iterate through sorted sprites
  i32 orderBufferPointer = 0;
  while (ctx->orderBuffer[orderBufferPointer] != -1) 
  {
    i32 oamIndex = ctx->orderBuffer[orderBufferPointer];
        
    u8 spriteYraw = ppuOAMread(oamIndex + 0); 
    u8 spriteXraw = ppuOAMread(oamIndex + 1);
    u8 tileNum    = ppuOAMread(oamIndex + 2);    
    u8 attributes = ppuOAMread(oamIndex + 3);  

    // - - - Adjust X and Y positions (Game Boy specific offsets)
    i32 spriteScreenX = spriteXraw - 8;
    i32 spriteScreenY = spriteYraw - 16;

    // - - - Skip if sprite is entirely off-screen horizontally
    if (spriteScreenX >= SCREEN_WIDTH || spriteScreenX + 8 <= 0) 
    {
      orderBufferPointer++;
      continue;
    }

    // - - - Calculate tile index for 8x16 sprites
    u8 actualTileNum    = tileNum;
    i32 tileRowInSprite = LY - spriteScreenY; 
    
    if (ppuIsYFlipped(attributes)) tileRowInSprite = (spriteSize - 1) - tileRowInSprite;

    if (spriteSize == 16) 
    {
      if (tileRowInSprite >= 8)  actualTileNum = tileNum | 0x01;  
      else                       actualTileNum = tileNum & 0xFE; 
      tileRowInSprite %= 8; 
    }

    u16  tileDataAddr   = 0x8000 + (actualTileNum * 16) + (tileRowInSprite * 2);
    u8   lo             = ppuVRAMread(tileDataAddr); 
    u8   hi             = ppuVRAMread(tileDataAddr + 1); 
    u32* palette        = BIT(attributes, 4) ? ctx->objectPalette1 : ctx->objectPalette0;

    // - - - iterate trhough 8 pixels of the sprite tile
    for (i32 p = 0; p < 8; ++p) 
    { 
      i32 currentPixelScreenX = spriteScreenX + p;

      // - - - Skip if pixel is off-screen horizontally
      if (currentPixelScreenX < 0 || currentPixelScreenX >= SCREEN_WIDTH) continue;

      u8 bitPosInByte = ppuIsXFlipped(attributes) ? p : (7 - p);
      u8 colorId      = ppuGetColorIdBits(bitPosInByte, lo, hi);

      if (ppuIsTransparent(colorId)) continue; 

      // - - - Check sprite priority
      if (ppuIsAboveBG(attributes) || ppuIsBGTransparent(currentPixelScreenX, LY)) 
      { ctx->frameBuffer[currentPixelScreenX + (LY * SCREEN_WIDTH)] = palette[colorId]; }
    }
    orderBufferPointer++;
  }
}

