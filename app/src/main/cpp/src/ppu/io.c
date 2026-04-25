#include <cartridge/cartridge.h>
#include <ppu/ppu.h>
#include <ppu/ppuRegisters.h>
#include <ppu/dma.h>
#include <ppu/internal.h>
#include <bus.h>


// - - - Memory Access Helpers - - - 

u8 ppuVRAMRead(u16 ADDRESS)
{
  PpuContext* ctx  = ppuGetContext();
  u8          mode = STAT_GET_MODE(ctx);

  // - - - Vram is inaccessible during Pixel Transfer
  if (mode == PPU_MODE_DRAW) return OPEN_BUS_VALUE;
  return ctx->vram[ctx->vramBank][ADDRESS & VRAM_MASK];
}

void ppuVRAMWrite(u16 ADDRESS, u8 VALUE)
{
  PpuContext* ctx  = ppuGetContext();
  u8          mode = STAT_GET_MODE(ctx);

  // - - - Vram is inaccessible during Pixel Transfer
  if (mode == PPU_MODE_DRAW) return;
  ctx->vram[ctx->vramBank][ADDRESS & VRAM_MASK] = VALUE;
}

u8 ppuOAMRead(u16 ADDRESS)
{
  PpuContext* ctx  = ppuGetContext();
  u8          mode = STAT_GET_MODE(ctx);

  if (mode == PPU_MODE_OAM || mode == PPU_MODE_DRAW) return OPEN_BUS_VALUE;
  return ctx->oam[ADDRESS & OAM_MASK];
}

void ppuOAMWrite(u16 ADDRESS, u8 VALUE)
{
  PpuContext* ctx  = ppuGetContext();
  u8          mode = STAT_GET_MODE(ctx);

  if (dmaIsActive() || (mode != PPU_MODE_OAM && mode != PPU_MODE_DRAW)) 
  { 
    ctx->oam[ADDRESS - BUS_ADDR_OAM_START] = VALUE; 
    return;
  }

  if (mode == PPU_MODE_OAM || mode == PPU_MODE_DRAW) return;
  ctx->oam[ADDRESS - BUS_ADDR_OAM_START] = VALUE;
}

u8 ppuRead(u16 ADDRESS)
{
  PpuContext* ctx = ppuGetContext();
  const bool dmg = cartridgeGetContext()->mode == MODE_DMG_GAMEBOY;

  switch (ADDRESS)
  {
    case LCD_CONTROL_REG        : return ctx->lcdc;
    case LCD_STATUS_REG         : return ctx->stat | STAT_MASK;
    case SCROLL_Y_REG           : return ctx->scy;
    case SCROLL_X_REG           : return ctx->scx;
    case LCD_Y_REG              : return ctx->ly;
    case LCD_Y_COMPARE_REG      : return ctx->lyc;
    case BG_PALETTE_REG         : return ctx->bgp;
    case OBJ_PALETTE0_REG       : return ctx->obp0;
    case OBJ_PALETTE1_REG       : return ctx->obp1;
    case WINDOW_X_REG           : return ctx->wx;
    case WINDOW_Y_REG           : return ctx->wy;
    case VBK_REG                : return (dmg) ? OPEN_BUS_VALUE : (ctx->vramBank | 0xFE);
    case OBJ_PALLETE_INDEX_REG  : return (dmg) ? OPEN_BUS_VALUE : ctx->objPaletteIndex;
    case OBJ_PALLETE_DATA_REG   : return (dmg) ? OPEN_BUS_VALUE : ctx->objColorRam[ctx->objPaletteIndex & PALLETE_MASK];
    case BG_PALLETE_INDEX_REG   : return (dmg) ? OPEN_BUS_VALUE : ctx->bgPaletteIndex;

    case BG_PALLETE_DATA_REG    : 
      {
        // - - - CGB Palette RAM is blocked in mode 3
        if (!dmg && STAT_GET_MODE(ctx) == PPU_MODE_DRAW) return OPEN_BUS_VALUE;
        return (dmg) ? OPEN_BUS_VALUE : ctx->bgColorRam[ctx->bgPaletteIndex & PALLETE_MASK];
        break;
      }

    default:
      if (ADDRESS >= VRAM_START && ADDRESS <= VRAM_END) return ppuVRAMRead(ADDRESS);
      if (ADDRESS >= OAM_START  && ADDRESS <= OAM_END)  return ppuOAMRead(ADDRESS);
      return OPEN_BUS_VALUE;
  }
}

void ppuWrite(u16 ADDRESS, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();
  const bool dmg = cartridgeGetContext()->mode == MODE_DMG_GAMEBOY;

  switch (ADDRESS)
  {
    case LCD_CONTROL_REG: 
      {
        ctx->lcdc = VALUE; 
        if (!LCDC_ENABLED(ctx))
        {
          STAT_SET_MODE(ctx, PPU_MODE_HBLANK);
          ctx->ly = 0;
        }
        break;
      }

    case LCD_STATUS_REG: 
      {
        // - - - Bits 0-2 are read only
        ctx->stat = (VALUE & 0x78) | (ctx->stat & 0x07) | 0x80;
        break;
      }

    case SCROLL_Y_REG           : ctx->scy  = VALUE; break;
    case SCROLL_X_REG           : ctx->scx  = VALUE; break;
    case LCD_Y_COMPARE_REG      : ctx->lyc  = VALUE; break;
    case DMA_TRIGGER            : dmaStart(VALUE);   break;
    case BG_PALETTE_REG         : ctx->bgp  = VALUE; break;
    case OBJ_PALETTE0_REG       : ctx->obp0 = VALUE; break;
    case OBJ_PALETTE1_REG       : ctx->obp1 = VALUE; break;
    case WINDOW_X_REG           : ctx->wx   = VALUE; break;
    case WINDOW_Y_REG           : ctx->wy   = VALUE; break;
    case VBK_REG                : if (!dmg) { ctx->vramBank = VALUE & 0x01; } break;
    case OBJ_PALLETE_INDEX_REG  : if (!dmg) { ctx->objPaletteIndex = VALUE & PALLETE_MASK; } break;
    case OBJ_PALLETE_DATA_REG   : if (!dmg) { ctx->objColorRam[ctx->objPaletteIndex & PALLETE_MASK] = VALUE; } break;
    case BG_PALLETE_INDEX_REG   : if (!dmg) { ctx->bgPaletteIndex = VALUE; } break;
    case BG_PALLETE_DATA_REG    : 
      {
        if (!dmg && STAT_GET_MODE(ctx) != PPU_MODE_DRAW)
        {
          ctx->bgColorRam[ctx->bgPaletteIndex & PALLETE_MASK] = VALUE; 
          if (ctx->bgPaletteAuto) 
          {
            // - - - Increment onlyt the 6-bit index, keeping bit 7
            u8 newIndex = (ctx->bgPaletteIndex + 1) & PALLETE_MASK;
            ctx->bgPaletteIndex = (ctx->bgPaletteIndex & 0x80) | newIndex;
          }
        }
        break;
      }

    default:
      if (ADDRESS >= VRAM_START && ADDRESS <= VRAM_END) ppuVRAMWrite(ADDRESS, VALUE);
      else if (ADDRESS >= OAM_START && ADDRESS <= OAM_END) ppuOAMWrite(ADDRESS, VALUE);
      break;
  }
}
