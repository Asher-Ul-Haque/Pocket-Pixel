#include "cartridge/cartridge.h"
#include "ppu/internal.h"
#include <ppu/ppu.h>
#include <bus.h>

/// @brief: Helper to determine if the PPU is currently drawing a line or scanning memory and should block CPU read/write access
bool ppuIsLcdEnabled(void)
{
  PpuContext* ctx = ppuGetContext();
  return (ctx->registers.lcdc & 0x80) != 0; // - - - LCDC Bit 7: LCD/PPU Enable
}


// - - - Vram - - - 

u8 ppuReadVram(u16 ADDR)
{
  PpuContext* ctx = ppuGetContext();

  /// @brief: When the PPU is rendering pixels (Mode 3), VRAM is completely inaccessible to the CPU and reads return 0xFF.
  if (ppuIsLcdEnabled() && ctx->mode == PPU_MODE_DRAWING) return OPEN_BUS_VALUE;

  u16 offset  = ADDR - BUS_ADDR_VRAM_START;
  u8  bank    = ctx->registers.vbk & 0x01;
  return ctx->vram[bank][offset];
}

void ppuWriteVram(u16 ADDR, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  ///@brief: Writes to VRAM during Mode 3 are entirely ignored.
  if (ppuIsLcdEnabled() && ctx->mode == PPU_MODE_DRAWING) return;

  u16 offset  = ADDR - BUS_ADDR_VRAM_START;
  u8  bank    = ctx->registers.vbk & 0x01;
  ctx->vram[bank][offset] = VALUE;
}


// - - - OAM RANGE INTERFACE (0xFE00 - 0xFE9F) - - - 

u8 ppuReadOam(u16 ADDR)
{
  PpuContext* ctx = ppuGetContext();

  /// @brief: OAM is inaccessible during Mode 2 (OAM Scan) and Mode 3 (Drawing). Reads during these blocked cycles return 0xFF.
  if (ppuIsLcdEnabled() && (ctx->mode == PPU_MODE_OAM_SCAN || ctx->mode == PPU_MODE_DRAWING))
  { return OPEN_BUS_VALUE;  }

  return ctx->oam[ADDR - BUS_ADDR_OAM_START];
}

void ppuWriteOam(u16 ADDR, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  /// @brief: Writes to OAM are ignored while the PPU scans or draws.
  if (ppuIsLcdEnabled() && (ctx->mode == PPU_MODE_OAM_SCAN || ctx->mode == PPU_MODE_DRAWING))
  { return; }

  ctx->oam[ADDR - BUS_ADDR_OAM_START] = VALUE;
}


// - - - CORE PPU I/O REGISTERS INTERFACE (0xFF40 - 0xFF55) - - - 

u8 ppuReadIo(u16 ADDR)
{
  PpuContext* ctx = ppuGetContext();

  switch (ADDR)
  {
    case REG_LCDC       : return ctx->registers.lcdc;
    case REG_STAT       : return ctx->registers.stat | 0x80; ///< Bit 7 is unused and always returns 1
    case REG_SCY        : return ctx->registers.scy;
    case REG_SCX        : return ctx->registers.scx;
    case REG_LY         : return ctx->registers.ly;
    case REG_LYC        : return ctx->registers.lyc;
    case REG_DMA        : return ctx->registers.dma;
    case REG_BGP        : return ctx->registers.bgp;   
    case REG_OBP_0      : return ctx->registers.obp0;  
    case REG_OBP_1      : return ctx->registers.obp1;  
    case REG_WY         : return ctx->registers.wy;
    case REG_WX         : return ctx->registers.wx;
    case REG_KEY_1      : return ctx->registers.key1 | 0x7E;  ///< Bits 1-6 are unused, always read high
    case REG_VRAM_BANK  : return ctx->registers.vbk  | 0xFE;  ///< Bits 1-7 are unused, always read high
    case REG_HDMA5      : return ctx->registers.hdma5;        ///< Returns remaining copy blocks

    default : return OPEN_BUS_VALUE;
  }
}

void ppuWriteIo(u16 ADDR, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  switch (ADDR)
  {
    case REG_LCDC:
      ctx->registers.lcdc = VALUE;
      break;

    /// @brief: Only bits 3-6 (Interrupt Select Enables) are writable by the CPU. Bits 0-2 reflect pure live hardware status flags.
    case REG_STAT:
      ctx->registers.stat &= 0x07;           ///< Retain read-only lower bits
      ctx->registers.stat |= (VALUE & 0x78); ///< Merge CPU written configuration bits
      break;

    case REG_SCY : ctx->registers.scy = VALUE; break;
    case REG_SCX : ctx->registers.scx = VALUE; break;
    case REG_LY  : break; /// @brief: LY is completely read-only
    case REG_LYC :
      ctx->registers.lyc = VALUE; 
      break;

    case REG_DMA:
      ctx->registers.dma  = VALUE;
      ctx->oamDma.source  = ((u16)VALUE) << OAM_DMA_SRC_SHIFT;
      ctx->oamDma.index   = OAM_DMA_START_INDEX;
      ctx->oamDma.active  = true; 
      break;

    case REG_WY : ctx->registers.wy = VALUE; break;
    case REG_WX : ctx->registers.wx = VALUE; break;

    /// @brief: CPU can only write to bit 0 to arm/disarm speed modifications
    case REG_KEY_1:
      ctx->registers.key1 &= 0xFE;           /// Strip old preparation state
      ctx->registers.key1 |= (VALUE & 0x01); /// Latch new configuration
      break;

    case REG_BGP   : ctx->registers.bgp  = VALUE; break; 
    case REG_OBP_0 : ctx->registers.obp0 = VALUE; break; 
    case REG_OBP_1 : ctx->registers.obp1 = VALUE; break;

    /// @brief: CGB: Latch VBK register
    case REG_VRAM_BANK:
      ctx->registers.vbk = VALUE;
      break;

    case REG_HDMA1: ctx->registers.hdma1 = VALUE; break;
    case REG_HDMA2: ctx->registers.hdma2 = VALUE; break;
    case REG_HDMA3: ctx->registers.hdma3 = VALUE; break;
    case REG_HDMA4: ctx->registers.hdma4 = VALUE; break;

    case REG_HDMA5:
      if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY) break;

      bool requestHblankMode          = (VALUE & HDMA_MODE_BIT_MASK) != 0;
      u8   requestedBlockLengthCount  = VALUE & HDMA_BLOCKS_LIMIT_MASK;

      // - - - Check if an active HDMA operation is already running mid-frame 
      if (ctx->cgbDma.active && !requestHblankMode)
      {
        ctx->cgbDma.active    = false;
        ctx->registers.hdma5 |= HDMA_MODE_BIT_MASK;
        break;
      }

      u16 systemSourcePointer     = (((u16) ctx->registers.hdma1) << HDMA_SRC_HIGH_SHIFT) | (ctx->registers.hdma2 & HDMA_SRC_LOW_ALIGN_MASK);
      u16 vramDestinationPointer  = (((u16)ctx->registers.hdma3 & HDMA_DEST_WINDOW_MASK) << HDMA_DEST_HIGH_SHIFT) | (ctx->registers.hdma4 & HDMA_DEST_LOW_ALIGN_MASK);

      vramDestinationPointer += VRAM_START_ADDR;

      ctx->cgbDma.source      = systemSourcePointer;
      ctx->cgbDma.destination = vramDestinationPointer;
      ctx->cgbDma.blocksLeft  = requestedBlockLengthCount;

      // - - - Mode A: general purpose bust DMA 
      if (!requestHblankMode)
      {
        u16 absoluteTotalBlocksToCopy = (u16) requestedBlockLengthCount + PIXEL_COLOR_MASK;
        u16 rawBytesToProcessCount    = absoluteTotalBlocksToCopy * TILE_BYTES;

        for (u16 progressIndex = CGB_DMA_START_INDEX; progressIndex < rawBytesToProcessCount; progressIndex++)
        {
          u8 networkDataByte        = busRead(ctx->cgbDma.source);
          u16 optimizedVramOffset   = (ctx->cgbDma.destination - VRAM_START_ADDR) & (VRAM_BANK_SIZE - PIXEL_COLOR_MASK);
          u8 targetedActiveVramBank = ctx->registers.vbk & BUS_BANK_BIT_MASK;

          ctx->vram[targetedActiveVramBank][optimizedVramOffset] = networkDataByte;
          ctx->cgbDma.source++;
          ctx->cgbDma.destination++;
        }

        ctx->registers.hdma5  = HDMA_FINISHED_STATUS;
        ctx->cgbDma.active    = false;
      }

      // - - - MODE B: Horizontal blank dynamic taster dma 
      else 
      {
        ctx->cgbDma.active    = true;
        ctx->registers.hdma5  = requestedBlockLengthCount;
      }
      break;
  }
}


// - - - CGB PALETTE CRAM INTERFACE (0xFF68 - 0xFF6B) - - - 

u8 ppuReadCram(u16 ADDR)
{
  PpuContext* ctx = ppuGetContext();

  /// @brief - - - Palette CRAM is inaccessible during Mode 3 rendering.
  if (ppuIsLcdEnabled() && ctx->mode == PPU_MODE_DRAWING)
  { return OPEN_BUS_VALUE; }

  if (ADDR == REG_BG_PALETTE_INDEX)  return ctx->registers.bgpi | 0x40; /// Bit 6 always high
  if (ADDR == REG_OBJ_PALETTE_INDEX) return ctx->registers.obpi | 0x40; /// Bit 6 always high

  if (ADDR == REG_BG_PALETTE_DATA)
  {
    u8 index = ctx->registers.bgpi & 0x3F; // 64 byte absolute array index
    return ctx->bgPaletteRam[index];
  }
  
  if (ADDR == REG_OBJ_PALETTE_DATA)
  {
    u8 index = ctx->registers.obpi & 0x3F; // 64 byte absolute array index
    return ctx->objPaletteRam[index];
  }

  return 0xFF;
}

void ppuWriteCram(u16 ADDR, u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  /// @brief: Writes to Color Palette memory are discarded during Mode 3.
  if (ppuIsLcdEnabled() && ctx->mode == PPU_MODE_DRAWING)
  { return; }

  if (ADDR == REG_BG_PALETTE_INDEX)  { ctx->registers.bgpi = VALUE;  return; }
  if (ADDR == REG_OBJ_PALETTE_INDEX) { ctx->registers.obpi = VALUE; return; }

  if (ADDR == REG_BG_PALETTE_DATA)
  {
    u8 index = ctx->registers.bgpi & 0x3F;
    ctx->bgPaletteRam[index] = VALUE;

    /// - - -  Auto-Increment Feature: If bit 7 of the Index register is high, advance the internal pointer automatically after a data write
    if (ctx->registers.bgpi & 0x80)
    {
      u8 nextIndex = (index + 1) & 0x3F;
      ctx->registers.bgpi = 0x80 | nextIndex;
    }
    return;
  }

  if (ADDR == REG_OBJ_PALETTE_DATA)
  {
    u8 index = ctx->registers.obpi & 0x3F;
    ctx->objPaletteRam[index] = VALUE;

    // - - -  Auto-Increment Feature: If bit 7 of the Index register is high, advance the internal pointer automatically after a data write
    if (ctx->registers.obpi & 0x80)
    {
      u8 nextIndex = (index + 1) & 0x3F;
      ctx->registers.obpi = 0x80 | nextIndex;
    }
    return;
  }
}


void ppuExecuteSpeedSwitch(void)
{
  PpuContext* ctx = ppuGetContext();

  /// - - - @warning: Executed only when a switch was armed via memory (bit 0)  and the CPU subsequently hits a STOP opcode block.
  if (ctx->registers.key1 & 0x01)
  {
    ctx->registers.key1 ^= 0x80; // Toggle Double Speed Mode bit (bit 7)
    ctx->registers.key1 &= 0xFE; // Clear active preparation flag (bit 0)
  }
}
