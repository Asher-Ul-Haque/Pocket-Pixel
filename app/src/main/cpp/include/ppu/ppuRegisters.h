#pragma once

/**
 * @file ppuRegisters.h
 * @brief Bitfield definitions and macros for PPU I/O registers.
 * Based on hardware documentation for DMG and CGB modes.
*/

#include <utils/bitwise.h>


// - - - LCDC: LCD Control (0xFF40) - - -

#define LCDC_ENABLED(CTX)       BIT((CTX)->lcdc, 7)                     // LCD Power (0=off, 1=on) 
#define LCDC_WIN_MAP(CTX)      (BIT((CTX)->lcdc, 6) ? 0x9C00 : 0x9800) 
#define LCDC_WIN_ENABLED(CTX)   BIT((CTX)->lcdc, 5) 
#define LCDC_DATA_AREA(CTX)    (BIT((CTX)->lcdc, 4) ? 0x8000 : 0x8800) 
#define LCDC_BG_MAP(CTX)       (BIT((CTX)->lcdc, 3) ? 0x9C00 : 0x9800) 
#define LCDC_OBJ_SIZE(CTX)     (BIT((CTX)->lcdc, 2) ? 16 : 8)           // (0=8x8, 1=8x16) 
#define LCDC_OBJ_ENABLED(CTX)   BIT((CTX)->lcdc, 1) 

/** 
 * LCDC Bit 0 has different meanings
 * DMG: BG Enabled
 * CGB: BG & Window Master Priority (0=Sprites always on top) 
*/
#define LCDC_BG_WIN_EN(CTX)    BIT((CTX)->lcdc, 0) 


// - - - STAT: LCD Status (0xFF41) - -

#define STAT_MODE2_INT(CTX)    BIT((CTX)->stat, 5) /// OAM Interrupt Enable
#define STAT_MODE1_INT(CTX)    BIT((CTX)->stat, 4) /// V-Blank Interrupt Enable 
#define STAT_MODE0_INT(CTX)    BIT((CTX)->stat, 3) /// H-Blank Interrupt Enable 
#define STAT_LYC_FLAG(CTX)     BIT((CTX)->stat, 2) /// LYC=LY Comparison Signal
#define STAT_LYC_INT(CTX)      BIT((CTX)->stat, 6) /// LYC=LY Interrupt Enable
#define STAT_SET_LYC_FLAG(CTX)    ((CTX)->stat = (CTX)->stat | STAT_LYC_FLAG(CTX))
#define STAT_CLEAR_LYC_FLAG(CTX)  ((CTX)->stat = (CTX)->stat & ~STAT_LYC_FLAG(CTX))

// - - Mode bits 1-0 - - -

#define STAT_GET_MODE(CTX)     ((CTX)->stat & 0x03)
#define STAT_SET_MODE(CTX, M)  ((CTX)->stat = ((CTX)->stat & ~0x03) | (M))


// - - - CGB Palette Select (BCPS 0xFF68 / OCPS 0xFF6A) - - -

#define PAL_INDEX(REG)         ((REG) & 0x3F)   /// Bits 0-5: Palette RAM Index
#define PAL_AUTO_INC(REG)      BIT((REG), 7)    /// Bit 7: Auto-increment on write


// - - - CGB Attributes (VRAM Bank 1) - - -

/// @note When fetching BG tiles in CGB mode, attributes are read from Bank 1

#define ATTR_CGB_PAL(A)        ((A) & 0x07)          /// Bits 0-2: Palette number
#define ATTR_VRAM_BANK(A)      (BIT((A), 3) ? 1 : 0) /// Bit 3: Tile VRAM Bank
#define ATTR_H_FLIP(A)          BIT((A), 5)          /// Bit 5: Horizontal Flip
#define ATTR_V_FLIP(A)          BIT((A), 6)          /// Bit 6: Vertical Flip
#define ATTR_PRIORITY(A)        BIT((A), 7)          /// Bit 7: BG-to-OAM Priority
