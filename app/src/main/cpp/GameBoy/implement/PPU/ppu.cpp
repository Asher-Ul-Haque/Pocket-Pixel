#include "../../include/ppu.h"
#include "../../../GameBoyCore.h" 
#include "../../include/directMemAccess.h"
#include "../../include/interrupt.h"
#include "../../include/common.h"

static PPUContext   ctx;
static i32          speedMultiplier = 1; 
static u8           VRAM[0x2000]; 
static u8           OAM[0xA0];    
static u8           colorSchemeIndex = 0;

const u32*  getColorScheme          ()          { return colorSchemes[colorSchemeIndex]; }
void        setColorScheme          (u8 INDEX)  { colorSchemeIndex = INDEX % 8; }
PPUContext* ppuGetContext           ()          { return &ctx; }
i32         ppuGetCpuSpeedMultiplier()          { return speedMultiplier; }


// - - - Read and Write - - -

u8   ppuVRAMread (u16 ADDRESS)             { return VRAM[ADDRESS & 0x1FFF]; }
u8   ppuOAMread  (u16 ADDRESS)             { return OAM[ADDRESS & 0xFF]; }
void ppuVRAMwrite(u16 ADDRESS, u8 VALUE)   { VRAM[ADDRESS & 0x1FFF] = VALUE; }
void ppuOAMwrite (u16 ADDRESS, u8 VALUE)   { OAM[ADDRESS & 0xFF]    = VALUE; }


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
  memset(VRAM, 0, sizeof(VRAM));        
  memset(OAM,  0, sizeof(OAM));        

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


// - - - Register Access - - -

u8 ppuRead(u16 ADDRESS) 
{
  PPUContext* ctx = ppuGetContext();
  switch (ADDRESS) 
  {
    case 0xFF40: return ctx->lcdc;
    case 0xFF41: return ctx->stat;
    case 0xFF42: return ctx->scy;
    case 0xFF43: return ctx->scx;
    case 0xFF44: return ctx->ly;
    case 0xFF45: return ctx->lyc;
    case 0xFF46: return 0xFF; 
    case 0xFF47: return ctx->bgp;
    case 0xFF48: return ctx->obp0;
    case 0xFF49: return ctx->obp1;
    case 0xFF4A: return ctx->wy;
    case 0xFF4B: return ctx->wx;
    default:
      FORGE_LOG_ERROR("Attempted to read unknown PPU register: 0x%04X\n", ADDRESS);
      return 0xFF; 
  }
}

void ppuWrite(u16 ADDRESS, u8 VALUE) 
{
  PPUContext* ctx = ppuGetContext();
  switch (ADDRESS) 
  {
    // - - - LCDC
    case 0xFF40: 
      { 
        if (VALUE == ctx->lcdc) return;

        ctx->lcdc       = VALUE;
        bool wasEnabled = ctx->isEnabled;
        BIT_SET(ctx->isEnabled, 0, BIT(VALUE, 7));

        if (!ctx->isEnabled) 
        {
          ctx->ly                   = 0;
          ctx->scanlineCounter      = 0;
          ctx->windowInternalLine   = 0;
          BIT_SET(ctx->stat, 0, 0);
          BIT_SET(ctx->stat, 1, 0);
        }

        // - - - If LCD was disabled and now enabled, set mode to OAM (Mode 2)
        if (!wasEnabled && ctx->isEnabled) 
        {
          ppuUpdateStatMode(MODE_OAM);
          ppuHandleCoincidenceFlag(); 
        }
        break;
      }

    // - - - STAT
    case 0xFF41: 
      { 
        // - - - Only bits 3-6 are writable by CPU, bits 0-2 (mode) are read-only
        u8 readOnlyFlags    = ctx->stat & 0x07; 
        ctx->stat           = (VALUE & 0x78) | readOnlyFlags;
        ppuHandleCoincidenceFlag();
        break;
      }

    case 0xFF42: ctx->scy = VALUE; break; // - - - SCY
    case 0xFF43: ctx->scx = VALUE; break; // - - - SCX

    // - - - LY 
    case 0xFF44: 
      { 
        ctx->ly = 0;
        ppuHandleCoincidenceFlag(); 
        break;
      }

    // - - - lyc
    case 0xFF45: 
      {
        ctx->lyc = VALUE;
        ppuHandleCoincidenceFlag(); 
        break;
      }
    
    // - - - DMA 
    case 0xFF46: 
      { 
        dmaStart(VALUE);
        break;
      }

    // - - - BGP
    case 0xFF47: 
      {
        if (VALUE == ctx->bgp) return;
        ctx->bgp = VALUE;
        ppuCachePalette(ctx->backgroundPalette, getColorScheme(), VALUE);
        break;
      }

    // - - - OBP0
    case 0xFF48: 
      {
        if (VALUE == ctx->obp0) return;
        ctx->obp0 = VALUE;
        ppuCachePalette(ctx->objectPalette0, getColorScheme(), VALUE);
        break;
      }

    // - - - OBP1
    case 0xFF49: 
    {
      if (VALUE == ctx->obp1) return;
      ctx->obp1 = VALUE;
      ppuCachePalette(ctx->objectPalette1, getColorScheme(), VALUE);
      break;
    }
        
    case 0xFF4A: ctx->wy = VALUE; break; // - - - WY
    case 0xFF4B: ctx->wx = VALUE; break; // - - - WX
    default:
      FORGE_LOG_ERROR("Attempted to write unknown PPU register: 0x%04X = 0x%02X\n", ADDRESS, VALUE);
      break;
  }
}

// - - - Palette Caching
void ppuCachePalette(u32* CACHED_PALLETE, const u32* COLORS_SOURCE, u8 PALLETE) 
{
  CACHED_PALLETE[0] = COLORS_SOURCE[PALLETE & 0x3];
  CACHED_PALLETE[1] = COLORS_SOURCE[(PALLETE >> 2) & 0x3];
  CACHED_PALLETE[2] = COLORS_SOURCE[(PALLETE >> 4) & 0x3];
  CACHED_PALLETE[3] = COLORS_SOURCE[(PALLETE >> 6) & 0x3];
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

void doubleSpeed() 
{
  if (speedMultiplier == 1) 
  {
    speedMultiplier = 2;
    FORGE_LOG_INFO("Double Speed ON. CPU Multiplier: %d", speedMultiplier);
  } 
  else 
  {
    speedMultiplier = 1;
    FORGE_LOG_INFO("Double Speed OFF. CPU Multiplier: %d", speedMultiplier);
  }
}
