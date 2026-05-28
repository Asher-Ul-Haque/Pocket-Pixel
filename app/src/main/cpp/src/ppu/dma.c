#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <cartridge/cartridge.h>
#include <bus.h>

void ppuStepOamDma(void)
{
  PpuContext* ctx = ppuGetContext();

  if (!ctx->oamDma.active) return;

  u16 absoluteSourceAddress = ctx->oamDma.source + ctx->oamDma.index;
  u8  fetchedDataByte       = busRead(absoluteSourceAddress);

  ctx->oam[ctx->oamDma.index] = fetchedDataByte;
  ctx->oamDma.index++;

  if (ctx->oamDma.index >= OAM_SIZE)
  {
    ctx->oamDma.active = false;
    ctx->oamDma.index  = OAM_DMA_START_INDEX;
    ctx->oamDma.source = OAM_DMA_START_INDEX;
  }
}

void ppuCheckHblankDma(void)
{
  PpuContext* ctx = ppuGetContext();

  if (cartridgeGetContext()->mode == MODE_DMG_GAMEBOY) return;
  if (!ctx->cgbDma.active)                             return;

  // - - - A single HDMA blast transfers exactly one 16 byte aligned chunk
  for (u8 byteOffset = 0; byteOffset < TILE_BYTES; byteOffset++)
  {
    u8  sourceDataByte        = busRead(ctx->cgbDma.source);
    u16 targetVramOffset      = (ctx->cgbDma.destination - BUS_ADDR_VRAM_START) & (VRAM_BANK_SIZE - 1);
    u8  currentActiveVramBank = ctx->registers.vbk & BUS_BANK_BIT_MASK;

    ctx->vram[currentActiveVramBank][targetVramOffset] = sourceDataByte;

    ctx->cgbDma.source++;
    ctx->cgbDma.destination++;
  }

  // - - - HDMA5 stores length-1. It underflows to 0xFF when the transfer is entirely complete.
  ctx->registers.hdma5--;
  
  if (ctx->registers.hdma5 == 0xFF)
  {
    ctx->cgbDma.active = false;
  }
}
