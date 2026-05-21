#pragma once 
#include <common.h>
#include <ppu/internal.h>

/// @brief Pixel Metadata bit-field
typedef struct 
{
  u8 colorIndex : 2; ///< 0-3 index
  u8 paletteId  : 3; ///< 0-7 CGB palette
  u8 layer      : 1; ///< 0: BG/Win, 1: OBJ
  u8 unused     : 2; 
} PpuPixelMetadata;

/// @brief PPU pixel, value, the ppu provides a 'map' texture to the platform, the platform is responsible for translating to colors, add shader effects etc
typedef union 
{
  PpuPixelMetadata  bits;
  u8                raw;
} PpuPixel;

/// @brief PPU frame, the 'map' texture provided to the platform 
typedef struct PpuFrame 
{
  PpuPixel pixels[WIDTH * HEIGHT];

  union 
  {
    struct 
    {
      u16 bg  [CGB_PALETTE_COLOR_COUNT * CGB_PALETTE_COUNT];
      u16 obj [CGB_PALETTE_COLOR_COUNT * CGB_PALETTE_COUNT];
    } cgb;

    struct 
    {
      u8 bgp;
      u8 obp0;
      u8 obp1;
    } dmg;
  } palettes;
} PpuFrame;


/// @brief PPU state machine mode 
typedef enum 
{
  PPU_MODE_HBLANK   = 0,
  PPU_MODE_VBLANK   = 1,
  PPU_MODE_OAM_SCAN = 2,
  PPU_MODE_DRAWING  = 3
} PpuMode;

typedef struct PpuRegisters 
{
  u8 lcdc;            ///< 0xFF40
  u8 stat;            ///< 0xFF41
  u8 scy;             ///< 0xFF42
  u8 scx;             ///< 0xFF43
  u8 ly;              ///< 0xFF44
  u8 lyc;             ///< 0xFF45
  u8 dma;             ///< 0xFF46
  u8 bgp;             ///< 0xFF47
  u8 obp0;            ///< 0xFF48
  u8 obp1;            ///< 0xFF49
  u8 wy;              ///< 0xFF4A
  u8 wx;              ///< 0xFF4B
  u8 bgPaletteIndex;  ///< 0xFF68
  u8 objPaletteIndex; ///< 0xFF6A
} PpuRegisters;

typedef enum PpuRegisterAddr 
{
  REG_LCDC              = 0xFF40,
  REG_STAT              = 0xFF41,
  REG_SCY               = 0xFF42,
  REG_SCX               = 0xFF43,
  REG_LY                = 0xFF44,
  REG_LYC               = 0xFF45,
  REG_DMA               = 0xFF46,
  REG_BGP               = 0xFF47,
  REG_OBP_0             = 0xFF48,
  REG_OBP_1             = 0xFF49,
  REG_WY                = 0xFF4A,
  REG_WX                = 0xFF4B,
  REG_VRAM_BANK         = 0xFF4F,
  REG_BG_PALETTE_INDEX  = 0xFF68,
  REG_BG_PALETTE_DATA   = 0xFF69,
  REG_OBJ_PALETTE_INDEX = 0xFF6A,
  REG_OBJ_PALETTE_DATA  = 0xFF6B
} PpuRegisterAddr;

typedef struct PpuContext 
{
  // - - - Internal Memory 
  u8 vram[VRAM_BANK_COUNT][VRAM_BANK_SIZE];
  u8 oam [OAM_SIZE];

  PpuRegisters registers;

  // - - - CGB Palette Ram 
  u8 bgPaletteRam [PALETTE_RAM_SIZE];
  u8 objPaletteRam[PALETTE_RAM_SIZE];

  // - - - State Machine 
  PpuMode   mode;
  u32       dotCount;         ///< Dot within current scanline
  PpuFrame  currentFrame;     ///< The one being filled right now
  u8        vramBankSelect;   ///< Internal tracker for FF4F
  bool      frameReady;
} PpuContext;

void  ppuInit(void);
void  ppuTick(u32 DOTS);
u8    ppuRead(u16 ADDR);
void  ppuWrite(u16 ADDR, u8 VALUE);
void  ppuDmaTrigger(u8 SOURCE_HIGH_BYTE);

PpuContext* ppuGetContext(void);


// - - - Internal modular helpers - - - 


void ppuSetMode(PpuMode MODE);

void ppuUpdateStatLycFlag(void);

void ppuHandleLycCompareEdge(bool PREVIOUS_MATCH, bool CURRENT_MATCH);

void ppuHandleModeInterrupt(PpuMode MODE);

void ppuHandleLcdStateChange(u8 PREVIOUS_LCDC, u8 NEW_LCDC);

bool ppuIsLcdEnabled(void);

void ppuRenderFrame(void);

void ppuRenderBgLayer(void);

void ppuRenderWindowLayer(void);

void ppuRenderObjLayer(void);
