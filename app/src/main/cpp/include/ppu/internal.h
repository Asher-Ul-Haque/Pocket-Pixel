#pragma once
#include <ppu/ppu.h>

/**
 * @file internal.h
 * @brief Internal helper functions and definitions for the PPU module.
 * @note Not to be included outside of PPU implementation files.
*/

// - - - Internal module communication
void ppuPipelineTick    (void);
void ppuOamSearchTick   (void);
void ppuHandleLyc       (void);

// - - - Color management
u32 ppuGetColorDMG(u8 PALETTE, u8 COLOR_ID);
u32 ppuGetColorCGB(u8 PALETTE_IDX, u8 COLOR_ID, bool IS_OBJ);

void fifoPush(PpuFifo* FIFO, PpuPixel PIXEL);
PpuPixel fifoPop(PpuFifo* FIFO);

// - - - Helper macros
#define VRAM_START            0x8000
#define VRAM_END              0x9FFF
#define OAM_START             0xFE00
#define OAM_END               0xFE9F
#define VRAM_MASK             0x1FFF
#define OAM_MASK              0xFF
#define PALLETE_MASK          0x3F
#define STAT_MASK             0x80
#define LCD_CONTROL_REG       0xFF40
#define LCD_STATUS_REG        0xFF41
#define SCROLL_Y_REG          0xFF42
#define SCROLL_X_REG          0xFF43
#define LCD_Y_REG             0xFF44 
#define LCD_Y_COMPARE_REG     0xFF45
#define BG_PALETTE_REG        0xFF47
#define OBJ_PALETTE0_REG      0xFF48
#define OBJ_PALETTE1_REG      0xFF49
#define WINDOW_Y_REG          0xFF4A
#define WINDOW_X_REG          0xFF4B
#define VBK_REG               0xFF4F 
#define BG_PALLETE_INDEX_REG  0xFF68
#define BG_PALLETE_DATA_REG   0xFF69
#define OBJ_PALLETE_INDEX_REG 0xFF6A
#define OBJ_PALLETE_DATA_REG  0xFF6B
#define DMA_TRIGGER           0xFF46

// - - - Memory Map Addresses 
#define VRAM_BASE_ADDR             0x8000
#define VRAM_MAP_0_ADDR            0x9800
#define VRAM_MAP_1_ADDR            0x9C00
#define VRAM_TILE_DATA_0_ADDR      0x8800  /// Signed addressing area
#define VRAM_TILE_DATA_1_ADDR      0x8000  /// Unsigned addressing area

// - - - Addressing Logic
#define TILE_DATA_SIGNED_OFFSET    128     /// Offset for 0x8800-0x97FF
#define TILE_DATA_SIZE_BYTES       16      /// 8x8 pixels, 2bpp 
#define TILE_PIXEL_WIDTH           8       /// Width of one tile 
#define MAP_ROW_SIZE_TILES         32      /// Tiles per row in a map 

// - - - Timing and Thresholds
#define T_CYCLES_MODE_2            80      /// OAM Search duration
#define T_CYCLES_SCANLINE          456     /// Total line duration
#define T_CYCLE_STEP               2       /// Fetcher steps every 2 T-cycles
#define FIFO_REFILL_THRESHOLD      8       /// Min pixels before fetcher pauses
#define FIFO_MASK                  15      /// Mask for circular FIFO indexing (size 16)

// - - - Coordinate Adjustments
#define WINDOW_X_REG_BIAS          7       /// WX=7 is screen X=0 
#define SPRITE_X_REG_BIAS          8       /// Sprite X-reg 8 is screen X=0
#define SPRITE_Y_REG_BIAS          16      /// Sprite Y-reg 16 is screen Y=0
                                           
// - - - OAM
#define OAM_ENTRY_COUNT            40
#define MAX_SPRITES_SCANLINE       10
#define SPRITE_Y_OFFSET            16   /// Register Y - 16 = Screen Y 
#define SPRITE_X_OFFSET            8    /// Register X - 8 = Screen X 
#define SPRITE_SIZE_BYTES          4    /// Each OAM entry is 4 bytes
#define SPRITE_X_ATTR_OFFSET       1    /// X coordinate is at byte 1
#define SPRITE_TILE_ATTR_OFFSET    2    /// Tile index is at byte 2
#define SPRITE_FLAGS_ATTR_OFFSET   3    /// Attributes are at byte 3
                                        
#define RGB555_R_MASK              0x001F  
#define RGB555_G_MASK              0x03E0  
#define RGB555_B_MASK              0x7C00  
#define RGB555_G_SHIFT             5       
#define RGB555_B_SHIFT             10      

#define ARGB_ALPHA_OPAQUE          0xFF000000
#define CGB_PALETTE_SIZE           8       /// 8 Background, 8 Object
#define COLORS_PER_PALETTE         4       /// 4 colors per palette 
                                           
// - - - Hardware defaults
#define DEFAULT_LCDC               0x91    /// LCD On, BG & Window enabled, Obj enabled, 8x8 sprites 
#define DEFAULT_STAT               0x85    /// LY=LYC interrupt enabled, Mode 2 OAM interrupt enabled

#define SCREEN_PIXELS_X     160
#define SCREEN_PIXELS_Y     144
#define VBLANK_END_LINE     153
