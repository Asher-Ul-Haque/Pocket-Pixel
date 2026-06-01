#include <bus.h>
#include <cartridge/memoryBankController.h>
#include <cartridge/cartridge.h>
#include <common.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

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
    ctx.mode = MODE_CGB_GAMEBOY; 
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
  mapperResetDefaults();

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
        if (ctx.hasRTC)
        {
          u32 rtcBlockSize = 64;
          u32 totalSize = ctx.externalRamSize + rtcBlockSize;
          u8* tempBuffer = (u8*)malloc(totalSize);
          if (tempBuffer)
          {
            memset(tempBuffer, 0, totalSize);
            if (ctx.fileIO->loadRamFromFile(tempBuffer, totalSize))
            {
              // - - - Unpack standard SRAM progress arrays
              memcpy(ctx.externalRamData, tempBuffer, ctx.externalRamSize);
              
              // - - - Unpack structural raw RTC states
              u32 rtcOffset = ctx.externalRamSize;
              u32 sec       = 0;
              u32 min       = 0; 
              u32 hr        = 0;
              u32 dy        = 0;
              u32 halt      = 0;
              u32 carry     = 0;
              u64 timestamp = 0;

              memcpy(&sec, tempBuffer + rtcOffset + 0, 4);
              memcpy(&min, tempBuffer + rtcOffset + 4, 4);
              memcpy(&hr, tempBuffer + rtcOffset + 8, 4);
              memcpy(&dy, tempBuffer + rtcOffset + 12, 4);
              memcpy(&halt, tempBuffer + rtcOffset + 16, 4);
              memcpy(&carry, tempBuffer + rtcOffset + 20, 4);
              memcpy(&timestamp, tempBuffer + rtcOffset + 24, 8);

              ctx.mapper.mbc3.rtcSeconds      = (u8)sec;
              ctx.mapper.mbc3.rtcMinutes      = (u8)min;
              ctx.mapper.mbc3.rtcHours        = (u8)hr;
              ctx.mapper.mbc3.rtcDays         = (u16)dy;
              ctx.mapper.mbc3.rtcHalt         = (halt != 0);
              ctx.mapper.mbc3.rtcDayCarry     = (carry != 0);
              ctx.mapper.mbc3.lastSystemTime  = (time_t)timestamp;

              // - - - Unpack Latched metadata values
              u32 l_sec   = 0;
              u32 l_min   = 0;
              u32 l_hr    = 0;
              u32 l_dy    = 0;
              u32 l_halt  = 0;
              u32 l_carry = 0;
              u32 latched = 0;
              memcpy(&l_sec, tempBuffer + rtcOffset + 32, 4);
              memcpy(&l_min, tempBuffer + rtcOffset + 36, 4);
              memcpy(&l_hr, tempBuffer + rtcOffset + 40, 4);
              memcpy(&l_dy, tempBuffer + rtcOffset + 44, 4);
              memcpy(&l_halt, tempBuffer + rtcOffset + 48, 4);
              memcpy(&l_carry, tempBuffer + rtcOffset + 52, 4);
              memcpy(&latched, tempBuffer + rtcOffset + 56, 4);

              ctx.mapper.mbc3.latchedSeconds  = (u8)l_sec;
              ctx.mapper.mbc3.latchedMinutes  = (u8)l_min;
              ctx.mapper.mbc3.latchedHours    = (u8)l_hr;
              ctx.mapper.mbc3.latchedDays     = (u16)l_dy;
              ctx.mapper.mbc3.latchedHalt     = (l_halt != 0);
              ctx.mapper.mbc3.latchedDayCarry = (l_carry != 0);
              ctx.mapper.mbc3.latched         = (latched != 0);
            }
            free(tempBuffer);
          }
        }
        else
        {
          ctx.fileIO->loadRamFromFile(ctx.externalRamData, ctx.externalRamSize);
        }
      }
    }
  }

  ctx.initialized = true;
  cartridgePrintMetadata();
  return true;
}

void cartridgeTick(void)
{
  // Live clock ticks execute programmatically upon controller mapping cycles
}

void cartridgeFlushRAM(void)
{
  if (!ctx.initialized) return;

  if (!ctx.hasBattery) return;
  if (!ctx.ramDirty)   return;
  if (!ctx.fileIO || !ctx.fileIO->saveRamToFile) return;

  if (ctx.mapperType == MAPPER_MBC2)
  {
    ctx.fileIO->saveRamToFile(ctx.mapper.mbc2.ram, RAM_SIZE_MBC2);
    ctx.ramDirty = false;
    return;
  }

  if (ctx.externalRamData)
  {
    if (ctx.hasRTC)
    {
      u32 rtcBlockSize  = 64;
      u32 totalSize     = ctx.externalRamSize + rtcBlockSize;
      u8* tempBuffer    = (u8*)malloc(totalSize);
      if (tempBuffer)
      {
        // - - - Copy static payload array
        memcpy(tempBuffer, ctx.externalRamData, ctx.externalRamSize);
        
        // - - - Append live timing variables
        u32 rtcOffset = ctx.externalRamSize;
        u32 sec       = (u32)ctx.mapper.mbc3.rtcSeconds;
        u32 min       = (u32)ctx.mapper.mbc3.rtcMinutes;
        u32 hr        = (u32)ctx.mapper.mbc3.rtcHours;
        u32 dy        = (u32)ctx.mapper.mbc3.rtcDays;
        u32 halt      = ctx.mapper.mbc3.rtcHalt ? 1 : 0;
        u32 carry     = ctx.mapper.mbc3.rtcDayCarry ? 1 : 0;
        u64 timestamp = (u64)ctx.mapper.mbc3.lastSystemTime;

        memcpy(tempBuffer + rtcOffset + 0, &sec, 4);
        memcpy(tempBuffer + rtcOffset + 4, &min, 4);
        memcpy(tempBuffer + rtcOffset + 8, &hr, 4);
        memcpy(tempBuffer + rtcOffset + 12, &dy, 4);
        memcpy(tempBuffer + rtcOffset + 16, &halt, 4);
        memcpy(tempBuffer + rtcOffset + 20, &carry, 4);
        memcpy(tempBuffer + rtcOffset + 24, &timestamp, 8);

        // - - - Append latched status registers
        u32 l_sec   = (u32)ctx.mapper.mbc3.latchedSeconds;
        u32 l_min   = (u32)ctx.mapper.mbc3.latchedMinutes;
        u32 l_hr    = (u32)ctx.mapper.mbc3.latchedHours;
        u32 l_dy    = (u32)ctx.mapper.mbc3.latchedDays;
        u32 l_halt  = ctx.mapper.mbc3.latchedHalt ? 1 : 0;
        u32 l_carry = ctx.mapper.mbc3.latchedDayCarry ? 1 : 0;
        u32 latched = ctx.mapper.mbc3.latched ? 1 : 0;

        memcpy(tempBuffer + rtcOffset + 32, &l_sec, 4);
        memcpy(tempBuffer + rtcOffset + 36, &l_min, 4);
        memcpy(tempBuffer + rtcOffset + 40, &l_hr, 4);
        memcpy(tempBuffer + rtcOffset + 44, &l_dy, 4);
        memcpy(tempBuffer + rtcOffset + 48, &l_halt, 4);
        memcpy(tempBuffer + rtcOffset + 52, &l_carry, 4);
        memcpy(tempBuffer + rtcOffset + 56, &latched, 4);
        
        // - - - Populate safety padding alignment boundary
        memset(tempBuffer + rtcOffset + 60, 0, 4);

        ctx.fileIO->saveRamToFile(tempBuffer, totalSize);
        free(tempBuffer);
        ctx.ramDirty = false;
      }
    }
    else
    {
      ctx.fileIO->saveRamToFile(ctx.externalRamData, ctx.externalRamSize);
      ctx.ramDirty = false;
    }
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
