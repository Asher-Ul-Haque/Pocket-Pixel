#pragma once 
#include <common.h>

#define WIDTH  160
#define HEIGHT 144

#define CGB_PALETTE_COUNT       8  
#define CGB_PALETTE_COLOR_COUNT 4

#define VRAM_BANK_SIZE  0x2000
#define VRAM_BANK_COUNT 2 
#define OAM_SIZE        160

#define PALETTE_RAM_SIZE            64
#define PALETTE_DATA_MASK           0x3F
#define PALETTE_AUTO_INCREMENT_BIT  0x80

#define TILE_SIDE     8
#define TILE_COUNT_X  16
#define TILE_COUNT_Y  24

#define PPU_DOTS_PER_SCANLINE 456
#define PPU_DOTS_PER_FRAME    70224

#define DOT_OAM_SCAN        80
#define DOTS_DRAWING        172
#define DOTS_HBLANK_DURATION (PPU_DOTS_PER_SCANLINE - DOT_OAM_SCAN - DOTS_DRAWING)
#define DOTS_TRANSFER_START DOT_OAM_SCAN
#define DOTS_HBLANK_START   (DOT_OAM_SCAN + DOTS_DRAWING)

#define LY_VBLANK_START 144 
#define LY_MAX          153
#define LY_PER_FRAME    154

#define BG_MAP_0_OFFSET 0x1800
#define BG_MAP_1_OFFSET 0x1C00
#define TILE_DATA_8000  0x0000
#define TILE_DATA_8800  0x1000
#define TILE_BYTES      16
#define TILE_MAP_WIDTH  32
#define TILE_MAP_MASK   0x1F
#define TILE_ROW_SHIFT  3

#define LCDC_BG_WIN_ENABLE_MASK 0x01
#define LCDC_OBJ_ENABLE_MASK    0x02
#define LCDC_OBJ_SIZE_MASK      0x04
#define LCDC_BG_TILE_MAP_MASK   0x08
#define LCDC_BG_WIN_DATA_MASK   0x10
#define LCDC_WIN_ENABLE_MASK    0x20
#define LCDC_WIN_TILE_MAP_MASK  0x40
#define LCDC_ENABLE_MASK        0x80

#define STAT_MODE_BITS_MASK      0x03
#define STAT_LYC_EQUALS_MASK     0x04
#define STAT_HBLANK_INT_MASK     0x08
#define STAT_VBLANK_INT_MASK     0x10
#define STAT_OAM_INT_MASK        0x20
#define STAT_LYC_INT_MASK        0x40
#define STAT_UNUSED_HIGH_BIT     0x80
#define STAT_WRITABLE_BITS_MASK  (STAT_HBLANK_INT_MASK | STAT_VBLANK_INT_MASK | STAT_OAM_INT_MASK | STAT_LYC_INT_MASK)

#define WINDOW_X_OFFSET         7
#define WINDOW_WX_MAX           166

#define SPRITE_COUNT              40
#define SPRITE_OAM_ENTRY_BYTES    4
#define SPRITE_WIDTH              8
#define SPRITE_HEIGHT_8           8
#define SPRITE_HEIGHT_16          16
#define SPRITE_MAX_PER_SCANLINE   10
#define SPRITE_Y_OFFSET           16
#define SPRITE_X_OFFSET           8

#define OAM_Y_OFFSET          0
#define OAM_X_OFFSET          1
#define OAM_TILE_OFFSET       2
#define OAM_ATTR_OFFSET       3

#define OBJ_ATTR_BG_PRIORITY_MASK   0x80
#define OBJ_ATTR_Y_FLIP_MASK        0x40
#define OBJ_ATTR_X_FLIP_MASK        0x20
#define OBJ_ATTR_DMG_PALETTE_MASK   0x10
#define OBJ_ATTR_VRAM_BANK_MASK     0x08
#define OBJ_ATTR_CGB_PALETTE_MASK   0x07

#define BOOT_LCDC 0x91
#define BOOT_STAT 0x85
#define BOOT_BGP  0xFC
#define BOOT_OBP0 0xFF
#define BOOT_OBP1 0xFF

#define TILE_LINE_BYTES          2
#define TILE_PIXEL_MASK          (TILE_SIDE - 1)
#define PIXEL_COLOR_MASK         0x01
#define DMG_OBJ_PALETTE_0        0
#define DMG_OBJ_PALETTE_1        1
#define LAYER_OBJECT             1
#define DEFAULT_VRAM_BANK        0
#define SPRITE_TILE_MASK_8X16    0xFE
#define INVALID_SPRITE_INDEX     0xFF
#define INVALID_SCREEN_X         0xFF
#define EMPTY_PIXEL_COLOR        0
#define DEFAULT_BG_PALETTE       0
#define DEFAULT_LAYER_BACKGROUND 0
