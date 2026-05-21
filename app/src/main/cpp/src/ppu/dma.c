#include <ppu/ppu.h>
#include <bus.h>

void ppuDmaTrigger(u8 SOURCE_HIGH_BYTE)
{
  PpuContext* ctx               = ppuGetContext();
  u16         sourceBaseAddress = (u16) SOURCE_HIGH_BYTE << 8;

  for (u16 offset = 0; offset < OAM_SIZE; ++offset)
  { ctx->oam[offset] = busRead(sourceBaseAddress + offset); }
}

void ppuHdmaTrigger(u8 VALUE)
{
  PpuContext* ctx = ppuGetContext();

  // 1. Calculate Source Address (Lower 4 bits are ignored)
  u16 source = ((u16)ctx->registers.hdma1 << 8) | (ctx->registers.hdma2 & 0xF0);

  // 2. Calculate Destination Address 
  // Upper 3 bits are ignored, lower 4 bits are ignored. 
  // It always writes to VRAM, so we baseline it at 0x8000.
  u16 dest = 0x8000 | (((u16)ctx->registers.hdma3 & 0x1F) << 8) | (ctx->registers.hdma4 & 0xF0);

  // 3. Calculate Transfer Length
  // The lower 7 bits of FF55 tell us how many 16-byte blocks to copy.
  u16 length = ((VALUE & 0x7F) + 1) * 0x10;

  // 4. Instant GDMA Copy
  // For now, we instantly copy the whole block regardless of if it's GDMA or HDMA mode.
  for (u16 i = 0; i < length; ++i)
  {
    u8 data = busRead(source + i);
    // Write directly into the currently active VRAM bank
    ctx->vram[ctx->vramBankSelect & 0x01][(dest + i) - 0x8000] = data;
  }

  // 5. Hardware Register Updates
  // After a transfer, the hardware updates the source/dest registers to the new offsets.
  ctx->registers.hdma1 = (u8)((source + length) >> 8);
  ctx->registers.hdma2 = (u8)((source + length) & 0xF0);
  ctx->registers.hdma3 = (u8)(((dest + length) >> 8) & 0x1F);
  ctx->registers.hdma4 = (u8)((dest + length) & 0xF0);
  
  // FF55 reading as 0xFF tells the game that the transfer is complete and inactive.
  ctx->registers.hdma5 = 0xFF; 
}
