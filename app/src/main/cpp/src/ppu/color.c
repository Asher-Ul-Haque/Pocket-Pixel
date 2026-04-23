#include <ppu/ppu.h>
#include <ppu/internal.h>
#include <ppu/ppuRegisters.h>
#include <cartridge/cartridge.h>

/**
 * @file color.c
 * @brief Color conversion and palette mapping logic for DMG and CGB.
*/


// - - - DMG Palette Mapping - - -

/**
 * @brief In DMG mode, we return the 2-bit shade index (0-3).
 * The Platform Layer will map these to actual colors.
 * @param PALETTE The 8-bit palette register (BGP, OBP0, or OBP1).
 * @param COLOR_ID The color index (0-3) to fetch from the palette.
 * @return The 2-bit shade index (0-3) corresponding to the COLOR_ID
*/
u32 ppuGetColorDMG(u8 PALETTE, u8 COLOR_ID) 
{
  // - - - Determine the 2-bit shade (0-3) from the palette byte
  return (PALETTE >> (COLOR_ID * 2)) & 0x03;
}


// - - - CGB Color Conversion - - -


/**
 * @brief Retrieves a 32-bit color from CGB Color RAM (CRAM).
 * @param PALETTE_INDEX The palette index (0-7).
 * @param COLOR_ID The color index within the palette (0-3).
 * @param IS_OBJ True if fetching from Object CRAM, false for Background.
 * @return The 32-bit ARGB color value corresponding to the specified palette and color index.
*/
u32 ppuGetColorCGB(u8 PALETTE_INDEX, u8 COLOR_ID, bool IS_OBJ) 
{
  PpuContext* ctx = ppuGetContext();

  // - - - Each color is 2 bytes, each palette is 4 colors (8 bytes total)
  u8  baseAddr  = (PALETTE_INDEX * COLORS_PER_PALETTE * 2) + (COLOR_ID * 2);
  u8* cram      = IS_OBJ ? ctx->objColorRam : ctx->bgColorRam;

  return cram[baseAddr] | (cram[baseAddr + 1] << 8);
}
