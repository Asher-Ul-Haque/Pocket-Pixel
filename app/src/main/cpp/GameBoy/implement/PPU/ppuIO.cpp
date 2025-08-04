#include "../../include/ppu.h" 
#include "../../include/directMemAccess.h"
#include "../../include/common.h"

// - - - Read and Write - - -

u8   ppuVRAMread (u16 ADDRESS)             { return ppuGetContext()->VRAM[ADDRESS & 0x1FFF]; }
u8   ppuOAMread  (u16 ADDRESS)             { return ppuGetContext()->OAM[ADDRESS & 0xFF]; }
void ppuVRAMwrite(u16 ADDRESS, u8 VALUE)   { ppuGetContext()->VRAM[ADDRESS & 0x1FFF] = VALUE; }
void ppuOAMwrite (u16 ADDRESS, u8 VALUE)   { ppuGetContext()->OAM[ADDRESS & 0xFF]    = VALUE; }


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
