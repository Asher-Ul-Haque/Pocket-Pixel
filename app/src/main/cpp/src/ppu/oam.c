#include <cartridge/cartridge.h>
#include <ppu/oam.h>

/**
 * @file oam.c
 * @brief Sprite selection and OAM search (Mode 2) logic
*/

static OamContext oamContext;
OamContext* oamGetContext(void) 
{ return &oamContext; }

void ppuOamSearchTick(void)
{
  PpuContext* ctx = ppuGetContext();
  OamContext* oam = oamGetContext();

  // - - - Mode 2 logic: This is usually called once per cycle in Mode 2, one sprite every 2 cycles
  if (ctx->dotClock % T_CYCLE_STEP != 0) return;

  u8 spriteIndex = ctx->dotClock / T_CYCLE_STEP;
  if (spriteIndex >= OAM_ENTRY_COUNT || oam->spriteCount >= MAX_SPRITES_SCANLINE) return;

  u16 oamAddr       = spriteIndex * SPRITE_SIZE_BYTES;
  u8  spriteY       = ctx->oam[oamAddr] - SPRITE_Y_OFFSET;
  u8  spriteHeight  = LCDC_OBJ_SIZE(ctx);

  // - - - Check if sprite is visible on the current scanline
  if (ctx->ly >= spriteY && ctx->ly < (spriteY + spriteHeight))
  {
    oam->scanLineSrpites[oam->spriteCount++] = (SpriteEntry)
    {
      .yPos      = spriteY,
      .xPos      = ctx->oam[oamAddr + SPRITE_X_ATTR_OFFSET],
      .tileIndex = ctx->oam[oamAddr + SPRITE_TILE_ATTR_OFFSET],
      .flags     = ctx->oam[oamAddr + SPRITE_FLAGS_ATTR_OFFSET],
      .oamIndex  = spriteIndex
    };
  }
}

void ppuOamResetSearch(void)
{
  OamContext* oam  = oamGetContext();
  oam->spriteCount = 0;
  memset(oam->scanLineSrpites, 0, sizeof(oam->scanLineSrpites));
}

/**
 * @brief Logic for the Pipeline to pause and fetch sprite pixels.
 * Mode 3 pauses the BG fetcher to fetch sprite tiles 
*/
void ppuOverlayDelaySprites(void)
{
  PpuContext* ctx = ppuGetContext();
  OamContext* oam = oamGetContext();

  if (!LCDC_OBJ_ENABLED(ctx)) return;

  u8 spriteHeight = LCDC_OBJ_SIZE(ctx);
  for (u8 i = 0; i < oam->spriteCount; ++i)
  {
    SpriteEntry*    sprite  = &oam->scanLineSrpites[i];
    u8              screenX = sprite->xPos - SPRITE_X_OFFSET; 

    // - - - Check if sprite starts at the current pixel being pushes to the FIFO 
    if (screenX == ctx->pixelsPushed)
    {
      // - - - 1. Calculate tile address 
      u8 row          = ctx->ly - sprite->yPos;

      // - - - Handle vertical flip (bit 6)
      if (ATTR_V_FLIP(sprite->flags)) row = (spriteHeight - 1) - row; 

      u8 tileIndex = sprite->tileIndex;
      if (spriteHeight == TILE_DATA_SIZE_BYTES) {
        // - - - 8x16 sprite mode: bit 0 is ignored, use two tiles (top/bottom)
        tileIndex &= 0xFE;
        if (row >= 8) {
          row -= 8;
          tileIndex++;
        }
      }
      
      u16 addr = VRAM_TILE_DATA_1_ADDR + (tileIndex * TILE_DATA_SIZE_BYTES) + (row * 2);


      // - - - 2. Fetch from VRAM (CGB bank check)
      u8 bank = (cartridgeGetContext()->mode == MODE_CGB_GAMEBOY) ? ATTR_VRAM_BANK(sprite->flags) : 0;
      u8 dataLow = ctx->vram[bank][addr & VRAM_MASK];
      u8 dataHigh = ctx->vram[bank][(addr + 1) & VRAM_MASK];


      // - - - 3. Push sprite pixels into the OBJ FIFO (handle horizontal flip and CGB palette)    
      for (u8 bit = 0; bit < TILE_PIXEL_WIDTH; ++bit) 
      {
        // - - - Handle Horizontal Flip (bit 5)
        u8 shift = ATTR_H_FLIP(sprite->flags) ? bit : (TILE_PIXEL_WIDTH - 1 - bit);
        u8 low      = (dataLow  >> shift) & 1;
        u8 high     = (dataHigh >> shift) & 1;
        u8 colorID  = (high << 1) | low;

        PpuPixel p = 
        {
          .pixel        = colorID,
          .palette      = ATTR_CGB_PAL(sprite->flags),
          .bgPriority   = ATTR_PRIORITY(sprite->flags),
          .priority     = 1
        };
        fifoPush(&ctx->objFifo, p);
      }
    }
  }
}
