#include "../../include/mappers.h"
#include "../../include/cartridge.h"

u8 mbc0Read(u16 ADDRESS)
{
  if (ADDRESS < cartridgeGetContext()->romSize) return cartridgeGetContext()->romData[ADDRESS];
  return 0xFF;
}

void mbc0Write(u16 ADDRESS, u8 VALUE)
{
  FORGE_LOG_FATAL("MBC0 : attempted write to ROM only address 0x%04X with 0x%02X", ADDRESS, VALUE);
}
