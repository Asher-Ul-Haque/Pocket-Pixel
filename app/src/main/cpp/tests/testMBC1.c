#include <utils/testManager.h>
#include <io/cartridge.h>

#include <string.h>
#include <stdlib.h>

/* ---------------------------- Test File I/O ----------------------------- */

static bool test_saveRamToFile(const u8* RAM_DATA, u32 RAM_SIZE)
{
  (void)RAM_DATA; (void)RAM_SIZE;
  return true;
}

static bool test_loadRamFromFile(u8* RAM_DATA_BUFFER, u32 BUFFER_SIZE)
{
  memset(RAM_DATA_BUFFER, 0, BUFFER_SIZE);
  return true;
}

static u32 test_getExpectedSaveSize(void)
{
  return 0;
}

static const CartridgeFileIO TEST_IO =
{
  .saveRamToFile       = test_saveRamToFile,
  .loadRamFromFile     = test_loadRamFromFile,
  .getExpectedSaveSize = test_getExpectedSaveSize
};

/* ---------------------------- ROM Builder ------------------------------- */

#define ROM_BANK_SIZE_BYTES 0x4000u

static u8 computeHeaderChecksum(const u8* rom)
{
  u8 checksum = 0x00;
  for (u16 addr = CHECKSUM_ADDR_MIN; addr <= CHECKSUM_ADDR_MAX; ++addr)
    checksum = (u8)(checksum - rom[addr] - 0x01);
  return checksum;
}

static u8* makeTestRom(u32 bankCount, u8 cartType, u8 romSizeCode, u8 ramSizeCode, u32* outRomSize)
{
  u32 romSize = bankCount * ROM_BANK_SIZE_BYTES;
  u8* rom = (u8*)malloc(romSize);
  memset(rom, 0, romSize);

  for (u32 b = 0; b < bankCount; ++b)
    memset(rom + (b * ROM_BANK_SIZE_BYTES), (int)(b & 0xFF), ROM_BANK_SIZE_BYTES);

  rom[0x0147] = cartType;
  rom[0x0148] = romSizeCode;
  rom[0x0149] = ramSizeCode;

  for (u16 a = CHECKSUM_ADDR_MIN; a <= CHECKSUM_ADDR_MAX; ++a) rom[a] = 0x00;
  rom[0x014D] = computeHeaderChecksum(rom);

  if (outRomSize) *outRomSize = romSize;
  return rom;
}

/* ------------------------------ Tests ----------------------------------- */

#define CART_TYPE_MBC1 0x01

static TestResult test_mbc1_rom_bank_switch_basic(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(8, CART_TYPE_MBC1, 0x02 /* 128KiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  cartridgeWrite(0x2000, 0x02);
  EXPECT_TO_BE((u8)0x02, cartridgeRead(0x4000));

  cartridgeWrite(0x2000, 0x00);
  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc1_ram_enable_and_write(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(4, CART_TYPE_MBC1, 0x01 /* 64KiB */, 0x02 /* 8KiB */, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  EXPECT_TO_BE((u8)0xFF, cartridgeRead(0xA000));

  cartridgeWrite(0x0000, 0x0A);
  cartridgeWrite(0xA000, 0x55);
  EXPECT_TO_BE((u8)0x55, cartridgeRead(0xA000));

  cartridgeWrite(0x0000, 0x00);
  EXPECT_TO_BE((u8)0xFF, cartridgeRead(0xA000));

  free(rom);
  return TEST_RESULT_PASSED;
}

/* ------------------------------ Main ------------------------------------ */

int main(void)
{
  registerTest(test_mbc1_rom_bank_switch_basic, "MBC1: ROM bank switching (2000-3FFF, 0->1 rule)");
  registerTest(test_mbc1_ram_enable_and_write,  "MBC1: RAM enable gate + RAM read/write");
  runTests();
  return 0;
}
