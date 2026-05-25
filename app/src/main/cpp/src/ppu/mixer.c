#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cartridge/cartridge.h>

static u16 ppuTranslateDmgColor(u8 COLOR_INDEX, u16 PALETTE_REGISTER_ADDR)
{
  u8 paletteRegisterValue = ppuReadIo(PALETTE_REGISTER_ADDR);
  u8 shiftAmount          = COLOR_INDEX << BIT_MASK_BASE;
  return (paletteRegisterValue >> shiftAmount) & STAT_MODE_BITS_MASK;
}

static u16 ppuGetCgbColor(bool IS_OBJ, u8 PALETTE_INDEX, u8 COLOR_INDEX)
{
  PpuContext* ctx = ppuGetContext();

  u16 absoluteCramAddress = ((u16) PALETTE_INDEX * COLOR_BYTES_PER_PALETTE) + ((u16) COLOR_INDEX * COLOR_BYTES_PER_INDEX);
  u8  dataLow             = IS_OBJ ? ctx->objPaletteRam[absoluteCramAddress] : ctx->bgPaletteRam[absoluteCramAddress];
  u8  dataHigh            = IS_OBJ ? ctx->objPaletteRam[absoluteCramAddress + BIT_MASK_BASE] : ctx->bgPaletteRam[absoluteCramAddress + BIT_MASK_BASE];

  return (((u16) dataHigh) << PIXEL_BIT_WIDTH) | dataLow;
}

void ppuStepPixelMixer(void)
{
  PpuContext* ctx   = ppuGetContext();
  bool        isDMG = cartridgeGetContext()->mode == MODE_DMG_GAMEBOY;

  if (ctx->spriteFetching) return;

  if ((ctx->registers.lcdc & LCDC_OBJ_ENABLE_MASK) != 0)
  {
    for (u8 scanIndex = FIFO_EMPTY_COUNT; scanIndex < ctx->scanlineOamBuffer.spriteCount; ++scanIndex)
    {
      u8  targetOamIndex  = ctx->scanlineOamBuffer.spriteIndices[scanIndex];
      u16 oamBase         = (u16) targetOamIndex * SPRITE_OAM_ENTRY_BYTES;
      u8  spriteX         = ctx->oam[oamBase + OAM_X_OFFSET];

      // THE FIX: Trigger normally, OR trigger instantly at screenX = 0 if the sprite is hanging off the left edge
      if ((((u16) ctx->screenX + 8) == (u16)spriteX) || 
          (ctx->screenX == 0 && spriteX > 0 && spriteX < 8))
      { 
        ppuInjectSpriteToFifo(scanIndex); 
      }
    }
  }

  if (ctx->bgFifo.count == FIFO_EMPTY_COUNT) return;

  u8 fineScrollOffset = ctx->registers.scx & TILE_PIXEL_MASK;
  if (ctx->droppedPixels < fineScrollOffset && !ctx->windowTriggered)
  {
    ctx->objFifo.pixels[ctx->objFifo.head].colorIndex = PIXEL_COLOR_TRANSPARENT;
    
    ctx->bgFifo.head  = (ctx->bgFifo.head  + BIT_MASK_BASE) % FIFO_CAPACITY;
    ctx->objFifo.head = (ctx->objFifo.head + BIT_MASK_BASE) % FIFO_CAPACITY;
    ctx->bgFifo.count--;
    ctx->objFifo.count--;
    ctx->droppedPixels++;
    return;
  }

  PpuPixel bgPixel  = ctx->bgFifo.pixels[ctx->bgFifo.head];
  PpuPixel objPixel = ctx->objFifo.pixels[ctx->objFifo.head];

  ctx->objFifo.pixels[ctx->objFifo.head].colorIndex = PIXEL_COLOR_TRANSPARENT;

  ctx->bgFifo.head  = (ctx->bgFifo.head  + BIT_MASK_BASE) % FIFO_CAPACITY;
  ctx->objFifo.head = (ctx->objFifo.head + BIT_MASK_BASE) % FIFO_CAPACITY;

  ctx->bgFifo.count--;
  ctx->objFifo.count--;

  bool chooseSprite = false;

  if ((ctx->registers.lcdc & LCDC_OBJ_ENABLE_MASK) != 0)
  {
    if (objPixel.colorIndex != PIXEL_COLOR_TRANSPARENT)
    {
      if (bgPixel.colorIndex == PIXEL_COLOR_TRANSPARENT)
      {
        chooseSprite = true;
      }
      else 
      {
        bool masterPriorityEnabled = (ctx->registers.lcdc & LCDC_MASTER_ENABLE_MASK) != 0;

        if (isDMG && !masterPriorityEnabled) chooseSprite = true; 
        else if (!isDMG && !masterPriorityEnabled) chooseSprite = true;
        else if (!isDMG && bgPixel.bgPriority) chooseSprite = false;
        else if (objPixel.spritePriority) chooseSprite = false;
        else chooseSprite = true;
      }
    }
  }

  u16 finalPixelColor = DMG_SHADE_LIGHTEST;
  if (chooseSprite)
  {
    if (isDMG)
    {
      u16 targetRegAddr = (objPixel.palette == DMG_OBJ_PALETTE_1) ? REG_OBP1_ADDR : REG_OBP0_ADDR;
      finalPixelColor   = ppuTranslateDmgColor(objPixel.colorIndex, targetRegAddr);
    }
    else 
    {
      finalPixelColor = ppuGetCgbColor(true, objPixel.palette, objPixel.colorIndex);
    }
  }
  else 
  {
    if (isDMG && ((ctx->registers.lcdc & LCDC_MASTER_ENABLE_MASK) == 0))
    {
      finalPixelColor = DMG_SHADE_LIGHTEST;
    }
    else if (isDMG)
    {
      finalPixelColor = ppuTranslateDmgColor(bgPixel.colorIndex, REG_BGP_ADDR);
    }
    else 
    {
      finalPixelColor = ppuGetCgbColor(false, bgPixel.palette, bgPixel.colorIndex);
    }
  }

  ppuPushPixelToScreen(ctx->screenX, ctx->registers.ly, finalPixelColor);
  ctx->screenX++;

  if (ctx->screenX >= MAX_SCREEN_X)
  {
    ctx->mode = PPU_MODE_HBLANK;
    ppuCheckHblankDma();
  }
}
