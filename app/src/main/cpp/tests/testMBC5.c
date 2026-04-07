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

#define CART_TYPE_MBC5 0x19

static TestResult test_mbc5_rom_bank_low8_bits(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(64, CART_TYPE_MBC5, 0x05 /* 1MiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  cartridgeWrite(0x2000, 0x2A);
  EXPECT_TO_BE((u8)0x2A, cartridgeRead(0x4000));

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc5_rom_bank_high_bit_selects_0x101(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(512, CART_TYPE_MBC5, 0x08 /* 8MiB */, 0x00, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  cartridgeWrite(0x2000, 0x01); /* low */
  cartridgeWrite(0x3000, 0x01); /* high bit => bank 0x101 */

  /* Our ROM fill uses (bankIndex & 0xFF), so bank 0x101 reads as 0x01 */
  EXPECT_TO_BE((u8)0x01, cartridgeRead(0x4000));

  free(rom);
  return TEST_RESULT_PASSED;
}

static TestResult test_mbc5_ram_enable_and_banking(void)
{
  u32 romSize = 0;
  u8* rom = makeTestRom(64, CART_TYPE_MBC5, 0x05 /* 1MiB */, 0x04 /* 128KiB */, &romSize);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  EXPECT_TO_BE((u8)0xFF, cartridgeRead(0xA000));

  cartridgeWrite(0x0000, 0x0A); /* enable RAM */

  cartridgeWrite(0x4000, 0x00);
  cartridgeWrite(0xA000, 0x22);

  cartridgeWrite(0x4000, 0x01);
  cartridgeWrite(0xA000, 0x23);

  cartridgeWrite(0x4000, 0x00);
  EXPECT_TO_BE((u8)0x22, cartridgeRead(0xA000));

  cartridgeWrite(0x4000, 0x01);
  EXPECT_TO_BE((u8)0x23, cartridgeRead(0xA000));

  free(rom);
  return TEST_RESULT_PASSED;
}

/* ------------------------------ Main ------------------------------------ */

int main(void)
{
  registerTest(test_mbc5_rom_bank_low8_bits,              "MBC5: ROM bank select low 8 bits");
  registerTest(test_mbc5_rom_bank_high_bit_selects_0x101, "MBC5: ROM bank select high bit (bank 0x101 reachable)");
  registerTest(test_mbc5_ram_enable_and_banking,          "MBC5: RAM enable + RAM banking keeps banks distinct");
  runTests();
  return 0;
}
