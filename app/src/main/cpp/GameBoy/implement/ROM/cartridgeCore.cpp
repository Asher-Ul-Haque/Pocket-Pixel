#include "../../include/cartridge.h"
#include "../../../GameBoyCore.h"
#include <cstring>
#include <ctime>

static CartContext ctx;

CartContext* cartridgeGetContext()
{ return &ctx; }

// - - - rom SIXE : 32 KB * (1 << N)
const u32 ROM_SIZE_MAP[] = 
  {
    32 * 1024,   // - - - 0x00: 32KB    (no banking)
    64 * 1024,   // - - - 0x01: 64KB    (4 banks)
    128 * 1024,  // - - - 0x02: 128KB   (8 banks)
    256 * 1024,  // - - - 0x03: 256KB   (16 banks)
    512 * 1024,  // - - - 0x04: 512KB   (32 banks)
    1024 * 1024, // - - - 0x05: 1MB     (64 banks)
    2048 * 1024, // - - - 0x06: 2MB     (128 banks)
    4096 * 1024, // - - - 0x07: 4MB     (256 banks)
    8192 * 1024  // - - - 0x08: 8MB     (512 banks)
  };

// - - - RAM Size: In bytes
const u32 RAM_SIZE_MAP[] = 
  {
    0,           
    2 * 1024,  
    8 * 1024, 
    32 * 1024,
    128 * 1024, 
    64 * 1024   
  };


// - - - lookup tables
static const char* ROM_TYPES[] = 
{
  "ROM ONLY",
  "MBC1",
  "MBC1+RAM",
  "MBC1+RAM+BATTERY",
  "0x04 ???",
  "MBC2",
  "MBC2+BATTERY",
  "0x07 ???",
  "ROM+RAM 1",
  "ROM+RAM+BATTERY 1",
  "0x0A ???",
  "MMM01",
  "MMM01+RAM",
  "MMM01+RAM+BATTERY",
  "0x0E ???",
  "MBC3+TIMER+BATTERY",
  "MBC3+TIMER+RAM+BATTERY 2",
  "MBC3",
  "MBC3+RAM 2",
  "MBC3+RAM+BATTERY 2",
  "0x14 ???",
  "0x15 ???",
  "0x16 ???",
  "0x17 ???",
  "0x18 ???",
  "MBC5",
  "MBC5+RAM",
  "MBC5+RAM+BATTERY",
  "MBC5+RUMBLE",
  "MBC5+RUMBLE+RAM",
  "MBC5+RUMBLE+RAM+BATTERY",
  "0x1F ???",
  "MBC6",
  "0x21 ???",
  "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
};

static const char* LICENSE_CODE[0xA5] = 
{
  [0x00] = "None",
  [0x01] = "Nintendo R&D1",
  [0x08] = "Capcom",
  [0x13] = "Electronic Arts",
  [0x18] = "Hudson Soft",
  [0x19] = "b-ai",
  [0x20] = "kss",
  [0x22] = "pow",
  [0x24] = "PCM Complete",
  [0x25] = "san-x",
  [0x28] = "Kemco Japan",
  [0x29] = "seta",
  [0x30] = "Viacom",
  [0x31] = "Nintendo",
  [0x32] = "Bandai",
  [0x33] = "Ocean/Acclaim",
  [0x34] = "Konami",
  [0x35] = "Hector",
  [0x37] = "Taito",
  [0x38] = "Hudson",
  [0x39] = "Banpresto",
  [0x41] = "Ubi Soft",
  [0x42] = "Atlus",
  [0x44] = "Malibu",
  [0x46] = "angel",
  [0x47] = "Bullet-Proof",
  [0x49] = "irem",
  [0x50] = "Absolute",
  [0x51] = "Acclaim",
  [0x52] = "Activision",
  [0x53] = "American sammy",
  [0x54] = "Konami",
  [0x55] = "Hi tech entertainment",
  [0x56] = "LJN",
  [0x57] = "Matchbox",
  [0x58] = "Mattel",
  [0x59] = "Milton Bradley",
  [0x60] = "Titus",
  [0x61] = "Virgin",
  [0x64] = "LucasArts",
  [0x67] = "Ocean",
  [0x69] = "Electronic Arts",
  [0x70] = "Infogrames",
  [0x71] = "Interplay",
  [0x72] = "Broderbund",
  [0x73] = "sculptured",
  [0x75] = "sci",
  [0x78] = "THQ",
  [0x79] = "Accolade",
  [0x80] = "misawa",
  [0x83] = "lozc",
  [0x86] = "Tokuma Shoten Intermedia",
  [0x87] = "Tsukuda Original",
  [0x91] = "Chunsoft",
  [0x92] = "Video system",
  [0x93] = "Ocean/Acclaim",
  [0x95] = "Varie",
  [0x96] = "Yonezawa/s’pal",
  [0x97] = "Kaneko",
  [0x99] = "Pack in soft",
  [0xA4] = "Konami (Yu-Gi-Oh!)"
};


// - - - | Functions | - - - 


// - - - lookups - - - 


// - - - license 
const char* getCartLicensee()
{
  if (ctx.metadata->newLicenseCode <= 0xA4) return LICENSE_CODE[ctx.metadata->licenseCode];
  else                                      return "UNKNOWN";
}

const char* getCartType()
{
  if (ctx.metadata->type <= 0x22) return ROM_TYPES[ctx.metadata->type];
  else                            return "UNKNOWN";
}


// - - - Load unload 
bool cartridgeLoad(u8* CARTRIDGE, u64 SIZE, CartridgeFileIO* IO)
{
  // - - - free everything
  cartridgeUnload();

  // - - - reset all fields
  memset(&ctx, 0, sizeof(CartContext));
  ctx.mapperType = MAPPER_UNKNOWN;

  FORGE_LOG_DEBUG("Trying to load a Cartridge of size : %d", SIZE);
  FORGE_ASSERT_MESSAGE(CARTRIDGE != NULL, "Cannot load a NULL CARTRIDGE");
  FORGE_ASSERT_MESSAGE(SIZE >= 0x150, "CARTRIDGE is too small. Not a valid Game Boy Cartridge (min size 0x150 for header).");

  ctx.romSize     = SIZE;
  ctx.romData     = CARTRIDGE;
  ctx.metadata    = (CartridgeMetadata*)(CARTRIDGE + 0x100);
  ctx.fileIO      = IO;
  ctx.ramDirty    = false;


  switch (ctx.metadata->type) 
  {
    // - - - ROM Only
    case 0x00: 
      ctx.mapperType = MAPPER_NONE;
      break;

    // - - - mbc1 
    case 0x01 ... 0x03: 
      ctx.mapperType                      = MAPPER_MBC1;
      ctx.mapperState.mbc1.ramEnabled     = false;
      ctx.mapperState.mbc1.currentRomBank = 1;
      ctx.mapperState.mbc1.currentRamBank = 0;
      ctx.mapperState.mbc1.romBankingMode = true;
      ctx.hasBattery                      = (ctx.metadata->type == 0x03); 
      break;


    // - - - mbc2 
    case 0x05 ... 0x06: 
      ctx.mapperType                      = MAPPER_MBC2;
      ctx.mapperState.mbc2.ramEnabled     = false;
      ctx.mapperState.mbc2.currentRomBank = 1;
      ctx.mapperState.mbc2.internalRam    = (u8*)malloc(256);
      FORGE_ASSERT_MESSAGE(ctx.mapperState.mbc2.internalRam != NULL, "Failed to allocate internal RAM for MBC2!");
      memset(ctx.mapperState.mbc2.internalRam, 0, 256);
      FORGE_LOG_INFO("Allocated 256 bytes of internal RAM for MBC2.");
      ctx.hasBattery                      = (ctx.metadata->type == 0x06);
      break;

    // - - - mbc3
    case 0x0F ... 0x13: 
      ctx.mapperType                          = MAPPER_MBC3;
      ctx.mapperState.mbc3.ramEnabled         = false;
      ctx.mapperState.mbc3.currentRomBank     = 1;
      ctx.mapperState.mbc3.currentRamBank     = 0;
      ctx.mapperState.mbc3.rtcSeconds         = 0;
      ctx.mapperState.mbc3.rtcMinutes         = 0;
      ctx.mapperState.mbc3.rtcHours           = 0;
      ctx.mapperState.mbc3.rtcDayLow          = 0;
      ctx.mapperState.mbc3.rtcDayHigh         = 0;
      ctx.mapperState.mbc3.latchedRTCseconds  = 0;
      ctx.mapperState.mbc3.latchedRTCminutes  = 0;
      ctx.mapperState.mbc3.latchedRTChours    = 0;
      ctx.mapperState.mbc3.latchedRTCdayLow   = 0;
      ctx.mapperState.mbc3.latchedRTCdayHigh  = 0;
      ctx.mapperState.mbc3.rtcLatched         = false;
      ctx.mapperState.mbc3.lastRTCsystemTime  = time(NULL);
      if (ctx.metadata->type == 0x0F || 
          ctx.metadata->type == 0x10 || 
          ctx.metadata->type == 0x13) 
      { ctx.hasBattery = true; }
      break;

    // - - - mbc5
    case 0x19 ... 0x1E: 
      ctx.mapperType                      = MAPPER_MBC5;
      ctx.mapperState.mbc5.ramEnabled     = false;
      ctx.mapperState.mbc5.currentRomBank = 1;
      ctx.mapperState.mbc5.currentRamBank = 0;
      if (ctx.metadata->type == 0x1B ||
          ctx.metadata->type == 0x1E)
      { ctx.hasBattery = true;}
      break;


    default:
      ctx.mapperType = MAPPER_UNKNOWN;
      FORGE_LOG_ERROR("Unsupported Cartridge Type: 0x%02X. Emulation may be incorrect.", ctx.metadata->type);
      break;
  }

  // - - - ALLOCATE EXTERNAL ram IF NEEDED
  if (ctx.metadata->ramSize < (sizeof(RAM_SIZE_MAP) / sizeof(RAM_SIZE_MAP[0])))
  {
    ctx.externalRamSize = RAM_SIZE_MAP[ctx.metadata->ramSize];
    if (ctx.externalRamSize > 0)
    {
      // - - - MBC2 has internal ram 
      if (ctx.mapperType != MAPPER_MBC2)
      {
        ctx.externalRamData= (u8*) malloc(ctx.externalRamSize);
        FORGE_ASSERT_MESSAGE(ctx.externalRamData, "Failed to allocated memory for external ram data");
        memset(ctx.externalRamData, 0, ctx.externalRamSize);
        FORGE_LOG_INFO("Allocated %d bytes of external RAM", ctx.externalRamSize);
      }
      else FORGE_LOG_INFO("MBC2 cartridge. EXternal ram size 0x%02X ignore, using internal RAM", ctx.metadata->ramSize);
    }
  }
  else 
  {
    FORGE_LOG_WARNING("Unkown RAM size code 0x%02X. ASSUMING no extneral ram");
    ctx.externalRamSize = 0;
  }

  FORGE_LOG_INFO("Cartridge Loaded:");
  FORGE_LOG_DEBUG("Title         : %s",          ctx.metadata->title);
  FORGE_LOG_DEBUG("Type          : 0x%02X (%s)", ctx.metadata->type, getCartType());
  FORGE_LOG_DEBUG("ROM Size      : %d KB",       ctx.romSize / 1024);
  FORGE_LOG_DEBUG("RAM Size      : %d KB",       ctx.externalRamSize / 1024); // Report external RAM
  FORGE_LOG_DEBUG("Licensee      : 0x%02X (%s)", ctx.metadata->licenseCode, getCartLicensee());
  FORGE_LOG_DEBUG("ROM Version   : 0x%02X",      ctx.metadata->version);

  // - - - Perform header checksum (not global checksum) - - -
  u8 headerChecksum = 0;
  for (u16 i = 0x0134; i <= 0x014C; i++) {
      headerChecksum = headerChecksum - ctx.romData[i] - 1;
  }
  bool checkSumPassed = (headerChecksum == ctx.metadata->checksum);

  if (checkSumPassed) { FORGE_LOG_INFO ("Header Checksum : 0x%02X (%s)\n", ctx.metadata->checksum, "PASSED"); }
  else                { FORGE_LOG_ERROR("Header Checksum : 0x%02X (%s)\n", ctx.metadata->checksum, "FAILED"); }

  // - - - attempt to load RAM data if battery backed 
  if (ctx.hasBattery)
  {
    u32 expectedSaveSize = 0;
    u8* ramToLoadPtr     = NULL;

    if (ctx.externalRamData && ctx.externalRamSize > 0)
    {
      expectedSaveSize = ctx.externalRamSize;
      ramToLoadPtr     = ctx.externalRamData;
    }
    else if (ctx.mapperType == MAPPER_MBC2 && ctx.mapperState.mbc2.internalRam)
    {
      expectedSaveSize = 256;
      ramToLoadPtr     = ctx.mapperState.mbc2.internalRam;
    }
    else if (ctx.mapperType == MAPPER_MBC3 && ctx.metadata->type == 0x0F)
    {
      TODO_COMMENT("MBC3 has no external ram, but has battery backed RTC");
    }

    if (ramToLoadPtr && expectedSaveSize > 0)
    {
      FORGE_LOG_INFO("Attempting to load ram");
      if (!ctx.fileIO->loadRamFromFile(ramToLoadPtr, expectedSaveSize))
      { FORGE_LOG_WARNING("failed to load ram, starting with a fresh state"); }
    }
  }

  return checkSumPassed;
}


void cartridgeUnload()
{
  if (ctx.ramDirty) cartridgeFlushRAM();
  if (ctx.externalRamData)
  {
    free(ctx.externalRamData);
    ctx.externalRamData = NULL;
    ctx.externalRamSize = 0;
    FORGE_LOG_INFO("Freed external RAM data");
  }

  if (ctx.mapperType == MAPPER_MBC2 && ctx.mapperState.mbc2.internalRam)
  {
    free(ctx.mapperState.mbc2.internalRam);
    ctx.mapperState.mbc2.internalRam = NULL;
    FORGE_LOG_INFO("Freed MBC2 internal RAM data");
  }

  ctx.romData   = NULL;
  ctx.romSize   = 0;
  ctx.metadata  = NULL;
  FORGE_LOG_INFO("cartridge unloaded");
}


// - - - Reading and Writing - - - 

// - - - read 
u8 cartridgeRead(u16 ADDRESS)
{
  switch (ctx.mapperType)
  {
    case MAPPER_NONE : return mbc0Read(ADDRESS);
    case MAPPER_MBC1 : return mbc1Read(ADDRESS);
    case MAPPER_MBC2 : return mbc2Read(ADDRESS);
    case MAPPER_MBC3 : return mbc3Read(ADDRESS);
    case MAPPER_MBC5 : return mbc5Read(ADDRESS);
    default :
      FORGE_LOG_FATAL("Attempted to read from unkown mapper type at address : 0x%04X", ADDRESS);
      return 0xFF;
  }
}

// - - - write 
void cartridgeWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ctx.mapperType)
  {
    case MAPPER_NONE : { mbc0Write(ADDRESS, VALUE); break; }
    case MAPPER_MBC1 : { mbc1Write(ADDRESS, VALUE); break; }
    case MAPPER_MBC2 : { mbc2Write(ADDRESS, VALUE); break; }
    case MAPPER_MBC3 : { mbc3Write(ADDRESS, VALUE); break; }
    case MAPPER_MBC5 : { mbc5Write(ADDRESS, VALUE); break; }
    default : 
      FORGE_LOG_FATAL("Attempted to write to unkown mapper type at address : 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}


// - - - Save and Load - - - 

void cartridgeFlushRAM()
{
  if (!ctx.ramDirty) return;
  if (!ctx.hasBattery || ctx.fileIO == NULL || ctx.fileIO->saveRamToFile == NULL)    return;    

  u8* ramToSavePtr  = NULL;
  u32 ramToSaveSize = 0;

  if (ctx.externalRamData && ctx.externalRamSize > 0)
  {
    ramToSavePtr  = ctx.externalRamData;
    ramToSaveSize = ctx.externalRamSize;
  }
  else if (ctx.mapperType == MAPPER_MBC2 && ctx.mapperState.mbc2.internalRam)
  {
    ramToSavePtr  = ctx.mapperState.mbc2.internalRam;
    ramToSaveSize = 256;
  }
  else return;

  if (ramToSavePtr && ramToSaveSize > 0)
  {
    if (!ctx.fileIO->saveRamToFile(ramToSavePtr, ramToSaveSize)) FORGE_LOG_ERROR("faile to flush RAM for");
  }

  ctx.ramDirty = false;
}


// - - - tick 
void cartridgeTickRTC()
{
  if (ctx.ramDirty)
  {
    pauseEmulator();
    //cartridgeFlushRAM();
    resumeEmulator();
  }
  if (ctx.mapperType != MAPPER_MBC3) return;

  // - - - do not update RTC if it's halted (bit 6 of DH is set)
  if (ctx.mapperState.mbc3.rtcDayHigh & 0x40) return;

  time_t    currentSystemTime   = time(NULL);
  long long elapsedSeconds      = (long long)currentSystemTime - ctx.mapperState.mbc3.lastRTCsystemTime;

  if (elapsedSeconds <= 0) return;

  ctx.mapperState.mbc3.rtcSeconds += elapsedSeconds;

  // - - - propagate carries for seconds, minutes, hours 
  if (ctx.mapperState.mbc3.rtcSeconds >= 60)
  {
    ctx.mapperState.mbc3.rtcMinutes += (ctx.mapperState.mbc3.rtcSeconds / 60);
    ctx.mapperState.mbc3.rtcSeconds %= 60;
  }
  if (ctx.mapperState.mbc3.rtcMinutes >= 60)
  {
    ctx.mapperState.mbc3.rtcHours   += (ctx.mapperState.mbc3.rtcMinutes / 60);
    ctx.mapperState.mbc3.rtcMinutes %= 60;
  }
  if (ctx.mapperState.mbc3.rtcHours >= 24)
  {
    u16 daysPassed                  = ctx.mapperState.mbc3.rtcHours / 24;
    ctx.mapperState.mbc3.rtcHours  %= 24;

    // - - - combine current day counter (9 bit)
    u16 currentDayCounter = (u16)((ctx.mapperState.mbc3.rtcDayHigh & 0x01) << 8) | ctx.mapperState.mbc3.rtcDayLow;
    currentDayCounter += daysPassed;

    if (currentDayCounter > 511)
    {
      ctx.mapperState.mbc3.rtcDayHigh |= 0x80;
      currentDayCounter %= 512;
    }
    ctx.mapperState.mbc3.rtcDayLow  = (u8)(currentDayCounter & 0xFF);
    ctx.mapperState.mbc3.rtcDayHigh = (ctx.mapperState.mbc3.rtcDayHigh & 0xFC) | ((currentDayCounter >> 8) & 0x01); 
  }

  ctx.mapperState.mbc3.lastRTCsystemTime = currentSystemTime;
}
