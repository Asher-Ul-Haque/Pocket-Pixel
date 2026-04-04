#include <io/cartridge.h>
#include <common.h>


// - - -  Internal state

static CartContext  ctx;


const CartContext* cartridgeGetContext(void)
{
  return &ctx;
}

bool cartridgeInit(const CartridgeFileIO* FILE_IO, const u8* ROM_DATA, const u32 ROM_SIZE)
{
  FORGE_ASSERT_MESSAGE(FILE_IO,                                 "[CARTRIDGE] : FILE_IO pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->loadRamFromFile,                "[CARTRIDGE] : FILE_IO->loadRamFromFile function pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->getExpectedSaveSize,            "[CARTRIDGE] : FILE_IO->getExpectedSaveSize function pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->saveRamToFile,                  "[CARTRIDGE] : FILE_IO->saveRamToFile function pointer is null");
  FORGE_ASSERT_MESSAGE(ROM_DATA,                                "[CARTRIDGE] : ROM_DATA pointer is null");
  FORGE_ASSERT_MESSAGE(ROM_SIZE > 0,                            "[CARTRIDGE] : ROM_SIZE must be greater than 0");
  FORGE_ASSERT_MESSAGE(ROM_SIZE >= sizeof(CartridgeMetadata),   "[CARTRIDGE] : Cartridge is too small/ Not a valid Game Boy Catridge");
  
  // - - - free everything
  cartridgeUnload(); 
  memset(&ctx, 0, sizeof(ctx));

  FORGE_LOG_DEBUG("[CARTRIDGE] : Trying to laod a Cartridge of size : %d", ROM_SIZE);
  ctx.fileIO            = (CartridgeFileIO*)FILE_IO;
  ctx.romSize           = ROM_SIZE;
  ctx.romData           = ROM_DATA;
  ctx.metadata          = (CartridgeMetadata*)(ctx.romData + CART_READ_OFFSET);
  ctx.externalRamSize   = cartridgeGetRamSize();

  // - - - Calculate the Header Checksum
  u8 checksum = 0x00;
  for (u16 addr = CHECKSUM_ADDR_MIN; addr <= CHECKSUM_ADDR_MAX; ++addr)
  {
    checksum = checksum - ctx.romData[addr] - 0x01;
  } 
  if (checksum != ctx.metadata->headerChecksum)
  {
    FORGE_LOG_ERROR("%s", "[CARTRIDGE] : Header Checksum Check Failed, ROM may be corrupted");
    return false;
  }

  // - - - Allocate external ram if needed 
  if (ctx.externalRamSize > 0)
  {
    ctx.externalRamData = (u8*)malloc(ctx.externalRamSize);
    if (ctx.externalRamData == NULL)
    {
      FORGE_LOG_FATAL("%s", "[CARTRIDGE] : Malloc failed for external RAM");
      return false;
    }
  }

  // - - - TODO: Handle MBC types and their specific RAM/RTC requirements
  ctx.initialized = true;
  cartridgePrintMetadata();
  return true;
}

void cartridgeTick(void)
{
  TODO_COMMENT("Mapper-specific ticking (MBC3 RTC, etc.)");
}

void cartridgeFlushRAM(void)
{
  FORGE_ASSERT_MESSAGE(ctx.initialized, "Cartridge not initialized");

  if (ctx.externalRamData && ctx.fileIO->saveRamToFile)
  {
    ctx.fileIO->saveRamToFile(
      ctx.externalRamData,
      ctx.externalRamSize
    );
  }
}

void cartridgeUnload(void)
{
  if (!ctx.initialized) return;

  cartridgeFlushRAM();

  if (ctx.externalRamData)
  {
    free(ctx.externalRamData);
    ctx.externalRamData = NULL;
  }

  if (ctx.romData)
  {
    free((u8*)ctx.romData);
    ctx.romData = NULL;
  }

  memset(&ctx, 0, sizeof(ctx));
  ctx.initialized = false;
}

u8 cartridgeRead(u16 ADDRESS)
{
  FORGE_ASSERT_MESSAGE(ctx.initialized, "Cartridge not initialized");
  TODO_COMMENT("Unhandled cartridgeRead mapping (MBC)");
  return ADDRESS;
}

void cartridgeWrite(u16 ADDRESS, u8 VALUE)
{
  FORGE_ASSERT_MESSAGE(ctx.initialized, "Cartridge not initialized");
  TODO_COMMENT("Unhandled cartridgeWrite (MBC / control registers)");
  ADDRESS += VALUE;
  VALUE += ADDRESS;
}

