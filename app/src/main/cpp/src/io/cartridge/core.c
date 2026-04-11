#include <bus.h>
#include <io/memoryBankController.h>
#include <io/cartridge.h>
#include <common.h>
#include <time.h>


// - - -  Internal state

static CartContext  ctx;


CartContext* cartridgeGetContext(void)
{ return &ctx; }

static void mapperResetDefaults(void)
{
  ctx.ramEnabled = false;
  ctx.ramDirty   = false;
  ctx.romBank    = 1; // - - - Bank 0 is fixed
  ctx.ramBank    = 0;

  memset(&ctx.mapper, 0, sizeof(ctx.mapper));

  switch (ctx.mapperType)
  {
    case MAPPER_NONE: break;

    case MAPPER_MBC1:
    case MAPPER_MBC1M:
      ctx.mapper.mbc1.romBankLow5 = 1;
      ctx.mapper.mbc1.bankHi2     = 0;
      ctx.mapper.mbc1.bankMode    = MBC1_BANK_MODE_ROM;
      break;

    case MAPPER_MBC2:
      ctx.mapper.mbc2.romBankLow4 = 1;
      break;
  
    case MAPPER_MBC3:
      ctx.mapper.mbc3.romBank7        = 1;
      ctx.mapper.mbc3.ramBankOrRtcReg = 0;
      ctx.mapper.mbc3.latchPrev       = 0;
      ctx.mapper.mbc3.lastSystemTime  = time(NULL);
      break;

    case MAPPER_MBC5:
      ctx.mapper.mbc5.romBank9 = 1;
      ctx.mapper.mbc5.ramBank4 = 0;
      break;

    case MAPPER_UNKNOWN:
    default:
      break;
  }
}

bool cartridgeInit(const CartridgeFileIO* FILE_IO, const u8* ROM_DATA, const u32 ROM_SIZE)
{
  FORGE_ASSERT_MESSAGE(FILE_IO,                                                   "[CARTRIDGE] : FILE_IO pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->loadRamFromFile,                                  "[CARTRIDGE] : FILE_IO->loadRamFromFile function pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->getExpectedSaveSize,                              "[CARTRIDGE] : FILE_IO->getExpectedSaveSize function pointer is null");
  FORGE_ASSERT_MESSAGE(FILE_IO->saveRamToFile,                                    "[CARTRIDGE] : FILE_IO->saveRamToFile function pointer is null");
  FORGE_ASSERT_MESSAGE(ROM_DATA,                                                  "[CARTRIDGE] : ROM_DATA pointer is null");
  FORGE_ASSERT_MESSAGE(ROM_SIZE > 0,                                              "[CARTRIDGE] : ROM_SIZE must be greater than 0");
  FORGE_ASSERT_MESSAGE(ROM_SIZE >= sizeof(CartridgeMetadata) + CART_READ_OFFSET,  "[CARTRIDGE] : Cartridge is too small/ Not a valid Game Boy Catridge");
  
  // - - - free everything
  cartridgeUnload(); 
  memset(&ctx, 0, sizeof(ctx));

  FORGE_LOG_DEBUG("[CARTRIDGE] : Trying to laod a Cartridge of size : %d", ROM_SIZE);
  ctx.romData           = ROM_DATA;
  ctx.romSize           = ROM_SIZE;
  ctx.fileIO            = (CartridgeFileIO*)FILE_IO;
  ctx.metadata          = (CartridgeMetadata*)(ctx.romData + CART_READ_OFFSET);
  ctx.externalRamSize   = cartridgeGetRamSize();
  ctx.mapperType        = cartridgeDetectMapperType();
  ctx.hasBattery        = cartridgeTypeHasBattery();
  ctx.hasRam            = ctx.externalRamSize > 0;
  ctx.hasRTC            = cartridgeTypeHasRTC();

  if (ctx.metadata->titleInfo.cgb.cgbFlag == CART_CGB_ONLY)
  {
    ctx.mode = MODE_CGB_ONLY_GAMEBOY;
  }
  else if (ctx.metadata->titleInfo.cgb.cgbFlag == CART_CGB_SUPPORTED)
  {
    ctx.mode = MODE_DMG_GAMEBOY; 
  }
  else
  {
    ctx.mode = MODE_DMG_GAMEBOY;
  }
  // - - - MBC2 has internal RAM 
  if (ctx.mapperType == MAPPER_MBC2)
  {
    ctx.externalRamSize = RAM_SIZE_MBC2;
    ctx.hasRam          = true;
  }

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
    // - - - Skip if MBC2, which has internal RAM
    if (ctx.mapperType != MAPPER_MBC2)
    {
      ctx.externalRamData = (u8*)malloc(ctx.externalRamSize);
      if (ctx.externalRamData == NULL)
      {
        FORGE_LOG_FATAL("%s", "[CARTRIDGE] : Malloc failed for external RAM");
        return false;
      }
      memset(ctx.externalRamData, 0, ctx.externalRamSize);
    }

    // - - - Load existing RAM data from file if battery-backed
    if (ctx.hasBattery) 
    {
      if (ctx.mapperType == MAPPER_MBC2)
      {
        ctx.fileIO->loadRamFromFile(ctx.mapper.mbc2.ram, RAM_SIZE_MBC2);
      }
      else if (ctx.externalRamData && ctx.externalRamSize > 0)
      {
        ctx.fileIO->loadRamFromFile(ctx.externalRamData, ctx.externalRamSize);
      }
    }
  }

  mapperResetDefaults();

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

  if (!ctx.hasBattery) return;
  if (!ctx.ramDirty)   return;
  if (ctx.externalRamData && ctx.fileIO->saveRamToFile)
  {
    ctx.fileIO->saveRamToFile(
      ctx.externalRamData,
      ctx.externalRamSize
    );
    ctx.ramDirty = false;
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

  memset(&ctx, 0, sizeof(ctx));
  ctx.initialized = false;
}

u8 cartridgeRead(u16 ADDRESS)
{
  FORGE_ASSERT_MESSAGE(ctx.initialized, "Cartridge not initialized");

  switch (ctx.mapperType)
  {
    case MAPPER_NONE: return mbc0Read(ADDRESS);
    case MAPPER_MBC1: return mbc1Read(ADDRESS);
    case MAPPER_MBC2: return mbc2Read(ADDRESS);
    case MAPPER_MBC3: return mbc3Read(ADDRESS);
    case MAPPER_MBC5: return mbc5Read(ADDRESS);
    default:
      FORGE_LOG_ERROR("[CARTRIDGE] : Unsupported mapper type %d", ctx.mapperType);
      return OPEN_BUS_VALUE;
  }
}

void cartridgeWrite(u16 ADDRESS, u8 VALUE)
{
  FORGE_ASSERT_MESSAGE(ctx.initialized, "Cartridge not initialized");

  switch (ctx.mapperType)
  {
    case MAPPER_NONE: mbc0Write(ADDRESS, VALUE); break;
    case MAPPER_MBC1: mbc1Write(ADDRESS, VALUE); break;
    case MAPPER_MBC2: mbc2Write(ADDRESS, VALUE); break;
    case MAPPER_MBC3: mbc3Write(ADDRESS, VALUE); break;
    case MAPPER_MBC5: mbc5Write(ADDRESS, VALUE); break;
    default:
      FORGE_LOG_ERROR("[CARTRIDGE] : Unsupported mapper type %d", ctx.mapperType);
      break;
  }
}

