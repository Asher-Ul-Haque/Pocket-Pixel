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

#define CART_TYPE_MBC2 0x05

static TestResult test_mbc2_bank0_behaves_as_bank1(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(16, CART_TYPE_MBC2, 0x03 /* 256KiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  cartridgeWrite(0x2100, 0x00);
  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc2_rom_bank_write_requires_A8_1(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(16, CART_TYPE_MBC2, 0x03 /* 256KiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  cartridgeWrite(0x2000, 0x02); /* A8=0 => ignored */
  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  cartridgeWrite(0x2100, 0x02); /* A8=1 => applied */
  EXPECT_TO_BE((u8)0x02, cartridgeRead(0x4000));

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc2_ram_enable_requires_A8_0(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(16, CART_TYPE_MBC2, 0x03 /* 256KiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  cartridgeWrite(0x0100, 0x0A); /* A8=1 => should NOT enable */
  cartridgeWrite(0xA000, 0x0B);
  EXPECT_TO_BE((u8)0xFF, cartridgeRead(0xA000));

  cartridgeWrite(0x0000, 0x0A); /* A8=0 => enable */
  cartridgeWrite(0xA000, 0x0B);
  EXPECT_TO_BE((u8)0xFB, cartridgeRead(0xA000)); /* 0xF0 | 0x0B */

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc2_ram_stores_low_nibble_only(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(16, CART_TYPE_MBC2, 0x03 /* 256KiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  cartridgeWrite(0x0000, 0x0A); /* enable RAM */

  cartridgeWrite(0xA000, 0xAB);
  EXPECT_TO_BE((u8)0xFB, cartridgeRead(0xA000));

  cartridgeWrite(0xA000, 0x04);
  EXPECT_TO_BE((u8)0xF4, cartridgeRead(0xA000));

  free(rom);
  return TEST_RESULT_PASSED;
}

/* ------------------------------ Main ------------------------------------ */

int main(void)
{
  registerTest(test_mbc2_bank0_behaves_as_bank1,       "MBC2: bank 0 behaves as bank 1");
  registerTest(test_mbc2_rom_bank_write_requires_A8_1, "MBC2: ROM bank writes require A8=1");
  registerTest(test_mbc2_ram_enable_requires_A8_0,     "MBC2: RAM enable requires A8=0");
  registerTest(test_mbc2_ram_stores_low_nibble_only,   "MBC2: RAM stores low nibble only (reads 0xF0|nibble)");
  runTests();
  return 0;
}
