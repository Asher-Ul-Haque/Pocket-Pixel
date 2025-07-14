#include "../include/ram.h"
#include "../../../ForgeLibrary/include/asserts.h"

static RAMctx ramCTX;

u8 wramRead(u16 ADDRESS)
{
  ADDRESS -= 0xC000;
  FORGE_ASSERT_MESSAGE(ADDRESS < 0x2000, "Invalid Wram address");

  return ramCTX.wram[ADDRESS];
}

void wramWrite(u16 ADDRESS, u8 VALUE)
{
  ADDRESS -= 0xC000;
  FORGE_ASSERT_MESSAGE(ADDRESS < 0x2000, "Invalid Wram address");

  ramCTX.wram[ADDRESS] = VALUE;
}

u8 hramRead(u16 ADDRESS)
{
  ADDRESS -= 0xFF80;
  FORGE_ASSERT_MESSAGE(ADDRESS < 0x80, "Invalid Hram address");

  return ramCTX.hram[ADDRESS];
}

void hramWrite(u16 ADDRESS, u8 VALUE)
{
  ADDRESS -= 0xFF80;
  FORGE_ASSERT_MESSAGE(ADDRESS < 0x80, "Invalid Wram address");

  ramCTX.hram[ADDRESS] = VALUE;
}

void loadRam(u8* BINARY)
{
  if (!BINARY) return;

  memcpy(ramCTX.wram, BINARY, sizeof(ramCTX.wram));
  memcpy(ramCTX.hram, BINARY + sizeof(ramCTX.wram), sizeof(ramCTX.hram));
}

u8* getRAM()
{  return ramCTX.wram; }
