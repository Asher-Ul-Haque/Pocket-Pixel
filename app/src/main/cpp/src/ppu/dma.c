#include <ppu/ppu.h>
#include <bus.h>

void ppuDmaTrigger(u8 SOURCE_HIGH_BYTE)
{
  PpuContext* ctx = ppuGetContext();
  u16 sourceBaseAddress = (u16) SOURCE_HIGH_BYTE << 8;

  for (u16 offset = 0; offset < OAM_SIZE; ++offset)
  { ctx->oam[offset] = busRead(sourceBaseAddress + offset); }
}
