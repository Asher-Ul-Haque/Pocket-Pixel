#include <io/cartridge.h>
#include <common.h>


const char* cartridgeGetTitle(void)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[CARTRIDGE] : not initialized");
  u8 cgbFlag = ctx->metadata->titleInfo.cgb.cgbFlag;

  if (cgbFlag == CART_CGB_SUPPORTED ||
      cgbFlag == CART_CGB_ONLY)
  { return ctx->metadata->titleInfo.cgb.title; }

  return ctx->metadata->titleInfo.oldTitle;
}

const char* cartridgeGetType(void)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[CARTRIDGE] : not initialized");

  switch (ctx->metadata->cartridgeType)
  {
    case 0x00: return "ROM ONLY";

    case 0x01: return "MBC1";
    case 0x02: return "MBC1+RAM";
    case 0x03: return "MBC1+RAM+BATTERY";

    case 0x04: return "0x04 ???";

    case 0x05: return "MBC2";
    case 0x06: return "MBC2+BATTERY";

    case 0x07: return "0x07 ???";

    case 0x08: return "ROM+RAM 1";
    case 0x09: return "ROM+RAM+BATTERY 1";

    case 0x0A: return "0x0A ???";

    case 0x0B: return "MMM01";
    case 0x0C: return "MMM01+RAM";
    case 0x0D: return "MMM01+RAM+BATTERY";

    case 0x0E: return "0x0E ???";

    case 0x0F: return "MBC3+TIMER+BATTERY";
    case 0x10: return "MBC3+TIMER+RAM+BATTERY 2";
    case 0x11: return "MBC3";
    case 0x12: return "MBC3+RAM 2";
    case 0x13: return "MBC3+RAM+BATTERY 2";

    case 0x14: return "0x14 ???";
    case 0x15: return "0x15 ???";
    case 0x16: return "0x16 ???";
    case 0x17: return "0x17 ???";
    case 0x18: return "0x18 ???";

    case 0x19: return "MBC5";
    case 0x1A: return "MBC5+RAM";
    case 0x1B: return "MBC5+RAM+BATTERY";
    case 0x1C: return "MBC5+RUMBLE";
    case 0x1D: return "MBC5+RUMBLE+RAM";
    case 0x1E: return "MBC5+RUMBLE+RAM+BATTERY";

    case 0x1F: return "0x1F ???";

    case 0x20: return "MBC6";

    case 0x21: return "0x21 ???";

    case 0x22: return "MBC7+SENSOR+RUMBLE+RAM+BATTERY";

    default:   return "UNKNOWN";
  }
}

#define KILOBYTE(x) ((x) * 1024)
#define MEGABYTE(x) ((x) * 1024 * 1024)

u32 cartridgeGetRomSize(void)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[CARTRIDGE] : not initialized");

  switch (ctx->metadata->romSize)
  {
    case 0x00: return KILOBYTE(32);  ///< no banking
    case 0x01: return KILOBYTE(64);  ///< 4 banks
    case 0x02: return KILOBYTE(128); ///< 8 banks
    case 0x03: return KILOBYTE(256); ///< 16 banks
    case 0x04: return KILOBYTE(512); ///< 32 banks
    case 0x05: return MEGABYTE(1);   ///< 64 banks
    case 0x06: return MEGABYTE(2);   ///< 128 banks
    case 0x07: return MEGABYTE(4);   ///< 256 banks
    case 0x08: return MEGABYTE(8);   ///< 512 banks

    default:
      FORGE_ASSERT_MESSAGE(false, "[CARTRIDGE] : Unknown ROM size code");
      return 0;
  }
}

u32 cartridgeGetRamSize(void)
{
  const CartContext* ctx = cartridgeGetContext();

  switch (ctx->metadata->ramSize)
  {
    case 0x00: return 0;
    case 0x01: return KILOBYTE(2);
    case 0x02: return KILOBYTE(8);
    case 0x03: return KILOBYTE(32);
    case 0x04: return KILOBYTE(128);
    case 0x05: return KILOBYTE(64);
    default  : return 0;
  }
}

#undef KILOBYTE
#undef MEGABYTE

const char* cartridgeGetLicensee(void)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[CARTRIDGE] : not initialized");

  u8 code = ctx->metadata->oldLicenseeCode;
  if (code == CART_OLD_LICENSEE_USE_NEW)
  {
    FORGE_LOG_WARNING("%s", "[CARTRIDGE] : Old licensee code indicates to use new licensee code, but new licensee codes cannot be found");
  }

  switch (code)
  {
    case 0x00: return "None";
    case 0x01: return "Nintendo R&D1";
    case 0x08: return "Capcom";
    case 0x13: return "Electronic Arts";
    case 0x18: return "Hudson Soft";
    case 0x19: return "b-ai";
    case 0x20: return "kss";
    case 0x22: return "pow";
    case 0x24: return "PCM Complete";
    case 0x25: return "san-x";
    case 0x28: return "Kemco Japan";
    case 0x29: return "seta";
    case 0x30: return "Viacom";
    case 0x31: return "Nintendo";
    case 0x32: return "Bandai";
    case 0x33: return "Ocean/Acclaim";
    case 0x34: return "Konami";
    case 0x35: return "Hector";
    case 0x37: return "Taito";
    case 0x38: return "Hudson";
    case 0x39: return "Banpresto";
    case 0x41: return "Ubi Soft";
    case 0x42: return "Atlus";
    case 0x44: return "Malibu";
    case 0x46: return "angel";
    case 0x47: return "Bullet-Proof";
    case 0x49: return "irem";
    case 0x50: return "Absolute";
    case 0x51: return "Acclaim";
    case 0x52: return "Activision";
    case 0x53: return "American sammy";
    case 0x54: return "Konami";
    case 0x55: return "Hi tech entertainment";
    case 0x56: return "LJN";
    case 0x57: return "Matchbox";
    case 0x58: return "Mattel";
    case 0x59: return "Milton Bradley";
    case 0x60: return "Titus";
    case 0x61: return "Virgin";
    case 0x64: return "LucasArts";
    case 0x67: return "Ocean";
    case 0x69: return "Electronic Arts";
    case 0x70: return "Infogrames";
    case 0x71: return "Interplay";
    case 0x72: return "Broderbund";
    case 0x73: return "sculptured";
    case 0x75: return "sci";
    case 0x78: return "THQ";
    case 0x79: return "Accolade";
    case 0x80: return "misawa";
    case 0x83: return "lozc";
    case 0x86: return "Tokuma Shoten Intermedia";
    case 0x87: return "Tsukuda Original";
    case 0x91: return "Chunsoft";
    case 0x92: return "Video system";
    case 0x93: return "Ocean/Acclaim";
    case 0x95: return "Varie";
    case 0x96: return "Yonezawa/s’pal";
    case 0x97: return "Kaneko";
    case 0x99: return "Pack in soft";
    case 0xA4: return "Konami (Yu-Gi-Oh!)";

    default:   return "UNKNOWN";
  }
}

void cartridgePrintMetadata(void)
{
#ifdef DEBUG
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx != NULL, "[CARTRIDGE] : context is NULL");
  FORGE_ASSERT_MESSAGE(ctx->metadata != NULL, "[CARTRIDGE] : metadata is NULL");

  FORGE_LOG_DEBUG("%s", "----- Cartridge Metadata -----");

  FORGE_LOG_DEBUG("Title           : %s", cartridgeGetTitle());
  FORGE_LOG_DEBUG("Type            : 0x%02X (%s)", ctx->metadata->cartridgeType, cartridgeGetType());

  FORGE_LOG_DEBUG("ROM Size Code   : 0x%02X (%u bytes)", 
                  ctx->metadata->romSize, cartridgeGetRomSize());

  FORGE_LOG_DEBUG("RAM Size Code   : 0x%02X (%u bytes)", 
                  ctx->metadata->ramSize, cartridgeGetRamSize());

  FORGE_LOG_DEBUG("Licensee        : 0x%02X (%s)", 
                  ctx->metadata->oldLicenseeCode, cartridgeGetLicensee());

  FORGE_LOG_DEBUG("CGB Flag        : 0x%02X", ctx->metadata->titleInfo.cgb.cgbFlag);
  FORGE_LOG_DEBUG("SGB Flag        : 0x%02X", ctx->metadata->sgbFlag);
  FORGE_LOG_DEBUG("Destination Code: 0x%02X", ctx->metadata->destinationCode);
  FORGE_LOG_DEBUG("ROM Version     : 0x%02X", ctx->metadata->maskRomVersion);

  FORGE_LOG_DEBUG("Header Checksum : 0x%02X", ctx->metadata->headerChecksum);
  FORGE_LOG_DEBUG("Global Checksum : 0x%04X", ctx->metadata->globalChecksum);

  FORGE_LOG_DEBUG("%s", "-------------------------------");
#endif
}
