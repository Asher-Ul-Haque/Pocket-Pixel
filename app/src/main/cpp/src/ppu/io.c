#include <bus.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>

u8 ppuRead(u16 ADDRESS)
{
  PpuContext* ctx = ppuGetContext();

  // - - - 1. Vram boundary access (blocked during mode 3 drawing)
  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END)
  {
    if (ctx->mode == PPU_MODE_DRAWING) return OPEN_BUS_VALUE;
    return ctx->vram[ctx->vramBankSelect & 0x01][ADDRESS - BUS_ADDR_VRAM_START];
  }

  // - - - 2. OAM Boundary Access constraints (blocked completley during Mode 3 drawing)
  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END)
  {
    if (ctx->mode == PPU_MODE_OAM_SCAN || ctx->mode == PPU_MODE_DRAWING)
    { return OPEN_BUS_VALUE; }
    return ctx->oam[ADDRESS - BUS_ADDR_OAM_START];
  }

  // - - - 3. Hardware I/O Registers 
  switch (ADDRESS)
  {
    case REG_LCDC : return ctx->registers.lcdc;
    case REG_STAT :
      {
        u8 statValue = STAT_UNUSED_HIGH_BIT;
        statValue |= (ctx->registers.stat & STAT_WRITABLE_BITS_MASK);
        statValue |= (ctx->mode & STAT_MODE_BITS_MASK);
        if (ctx->registers.ly == ctx->registers.lyc) statValue |= STAT_LYC_EQUALS_MASK;
        return statValue;
      }
    case REG_SCY  : return ctx->registers.scy;
    case REG_SCX  : return ctx->registers.scx;
    case REG_LY   : return ctx->registers.ly;
    case REG_LYC  : return ctx->registers.lyc;
    case REG_DMA  : return ctx->registers.dma;
    case REG_BGP  : return ctx->registers.bgp;
    case REG_OBP_0: return ctx->registers.obp0;
    case REG_OBP_1: return ctx->registers.obp1;
    case REG_WY   : return ctx->registers.wy;
    case REG_WX   : return ctx->registers.wx;

    // - - - CGB VRAM Bank Select 
    case REG_VRAM_BANK: return ctx->vramBankSelect | 0xFE;

    // - - - case BG Palettes 
    case REG_BG_PALETTE_INDEX: return ctx->registers.bgPaletteIndex;
    case REG_BG_PALETTE_DATA :
      {
        if (ctx->mode == PPU_MODE_DRAWING) return OPEN_BUS_VALUE;
        u8 index = ctx->registers.bgPaletteIndex & PALETTE_DATA_MASK;
        return ctx->bgPaletteRam[index];
      }

    // - - - CGB Obj palettes 
    case REG_OBJ_PALETTE_INDEX: return ctx->registers.objPaletteIndex;
    case REG_OBJ_PALETTE_DATA :
      {
        if (ctx->mode == PPU_MODE_DRAWING) return OPEN_BUS_VALUE;
        u8 index = ctx->registers.objPaletteIndex & PALETTE_DATA_MASK;
        return ctx->objPaletteRam[index];
      }

    default: break;
  }

  return OPEN_BUS_VALUE;
}

void ppuWrite(u16 ADDRESS, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  if (ADDRESS >= BUS_ADDR_VRAM_START && ADDRESS <= BUS_ADDR_VRAM_END)
  {
    if (ctx->mode == PPU_MODE_DRAWING) return;
    ctx->vram[ctx->vramBankSelect & 0x01][ADDRESS - BUS_ADDR_VRAM_START] = VALUE;
    return;
  }

  if (ADDRESS >= BUS_ADDR_OAM_START && ADDRESS <= BUS_ADDR_OAM_END)
  {
    if (ctx->mode == PPU_MODE_OAM_SCAN || ctx->mode == PPU_MODE_DRAWING) return;
    ctx->oam[ADDRESS - BUS_ADDR_OAM_START] = VALUE;
    return;
  }

  switch (ADDRESS)
  {
    case REG_LCDC:
      {
        const u8 previousLcdc = ctx->registers.lcdc;
        ctx->registers.lcdc = VALUE;
        ppuHandleLcdStateChange(previousLcdc, VALUE);
        break;
      }

    case REG_STAT:
      {
        ctx->registers.stat &= (u8) ~STAT_WRITABLE_BITS_MASK;
        ctx->registers.stat |= (VALUE & STAT_WRITABLE_BITS_MASK);
        ctx->registers.stat |= STAT_UNUSED_HIGH_BIT;
        ppuUpdateStatLycFlag();
        break;
      }

    case REG_SCY : ctx->registers.scy  = VALUE; break;
    case REG_SCX : ctx->registers.scx  = VALUE; break;
    case REG_LY  : break;
    case REG_LYC :
      {
        const bool previousMatch = ctx->registers.ly == ctx->registers.lyc;
        ctx->registers.lyc = VALUE;
        const bool currentMatch = ctx->registers.ly == ctx->registers.lyc;
        ppuUpdateStatLycFlag();
        ppuHandleLycCompareEdge(previousMatch, currentMatch);
        break;
      }
    case REG_DMA :
      ctx->registers.dma = VALUE;
      ppuDmaTrigger(VALUE);
      break;
    case REG_BGP   : ctx->registers.bgp  = VALUE; break;
    case REG_OBP_0 : ctx->registers.obp0 = VALUE; break;
    case REG_OBP_1 : ctx->registers.obp1 = VALUE; break;
    case REG_WY    : ctx->registers.wy   = VALUE; break;
    case REG_WX    : ctx->registers.wx   = VALUE; break;

    // - - - CGB Banking updates 
    case REG_VRAM_BANK: ctx->vramBankSelect = VALUE & 0x01; break;
    
    // - - - CGB Palette Ram Array Operations
    case REG_BG_PALETTE_INDEX: ctx->registers.bgPaletteIndex = VALUE; break;
    case REG_BG_PALETTE_DATA:
      {
        if (ctx->mode == PPU_MODE_DRAWING) break;

        u8 index = ctx->registers.bgPaletteIndex & PALETTE_DATA_MASK;
        ctx->bgPaletteRam[index] = VALUE;

        u8   paletteNum  = index / 8;
        u8   colorNum    = (index % 8) / 2;
        u16* targetColor = &ctx->currentFrame.palettes.cgb.bg[(paletteNum * 4) + colorNum];

        if    (index % 2 == 0) *targetColor = (*targetColor & 0xFF00) | VALUE;
        else                   *targetColor = (*targetColor & 0x00FF) | ((u16)VALUE << 8);

        if (ctx->registers.bgPaletteIndex & PALETTE_AUTO_INCREMENT_BIT)
        {
          ctx->registers.bgPaletteIndex = PALETTE_AUTO_INCREMENT_BIT | ((index + 1) & PALETTE_DATA_MASK);
        }
        break;
      }

    case REG_OBJ_PALETTE_INDEX: ctx->registers.objPaletteIndex = VALUE; break;
    case REG_OBJ_PALETTE_DATA :
      {
        if (ctx->mode == PPU_MODE_DRAWING) break;

        u8 index = ctx->registers.objPaletteIndex & PALETTE_DATA_MASK;
        ctx->objPaletteRam[index] = VALUE;

        u8    paletteNum  = index / 8;
        u8    colorNum    = (index % 8) / 2;
        u16*  targetColor = &ctx->currentFrame.palettes.cgb.obj[(paletteNum* 4) + colorNum];

        if    (index % 2 == 0) *targetColor = (*targetColor & 0xFF00) | VALUE;
        else                   *targetColor = (*targetColor & 0x00FF) | ((u16)VALUE << 8);

        if (ctx->registers.objPaletteIndex & PALETTE_AUTO_INCREMENT_BIT)
        {
          ctx->registers.objPaletteIndex = PALETTE_AUTO_INCREMENT_BIT | ((index + 1) & PALETTE_DATA_MASK);
        }
        break;
      }

    default: break;
  }
}
