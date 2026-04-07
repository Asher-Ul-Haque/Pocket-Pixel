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
#define BANK_0x10      0x10u

static TestResult test_mbc1m_mode1_maps_rom0_to_bank10_region(void)
{
  u32 romSize = 0;
  u32 bankCount = 0x20u; /* includes bank 0x10 */
  u8* rom = makeTestRom(bankCount, CART_TYPE_MBC1, 0x04 /* 512KiB */, 0x00, &romSize);

  /* Force your MBC1M heuristic: bank 0x10 logo == bank 0 logo */
  memcpy(rom + (BANK_0x10 * ROM_BANK_SIZE_BYTES) + NINTENDO_LOGO_OFFSET,
         rom + NINTENDO_LOGO_OFFSET,
         NINTENDO_LOGO_SIZE);

  /* checksum region not touched, but keep consistent */
  rom[0x014D] = computeHeaderChecksum(rom);

  EXPECT_TO_BE_TRUE(cartridgeInit(&TEST_IO, rom, romSize));

  /* mode 1 */
  cartridgeWrite(0x6000, 0x01);

  /* hi2 = 1 => ROM0 maps to bank 0x10 for MBC1M */
  cartridgeWrite(0x4000, 0x01);

  EXPECT_TO_BE((u8)0x10, cartridgeRead(0x0000));

  free(rom);
  return TEST_RESULT_PASSED;
}

/* ------------------------------ Main ------------------------------------ */

int main(void)
{
  registerTest(test_mbc1m_mode1_maps_rom0_to_bank10_region,
               "MBC1M: mode1 + hi2=1 maps ROM0 window to bank 0x10 region (auto-detect heuristic)");
  runTests();
  return 0;
}
