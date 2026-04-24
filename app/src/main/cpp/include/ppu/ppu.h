#pragma once

/**
 * @file ppu.h
 * @brief Pixel Processing Unit (PPU) core for DMG and CGB.
 * * Supports cycle-accurate Pixel FIFO fetching and CGB-specific 
 * memory banking/palettes.
*/

#include <common.h>

// - - - Constants - - -
#define SCREEN_WIDTH         160
#define SCREEN_HEIGHT        144
#define VBLANK_LINES         10
#define TOTAL_LINES          (SCREEN_HEIGHT + VBLANK_LINES)
#define TICKS_PER_LINE       456                                


// - - - PPU Modes - - -
typedef enum 
{
  PPU_MODE_HBLANK = 0, /// Mode 0
  PPU_MODE_VBLANK = 1, /// Mode 1
  PPU_MODE_OAM    = 2, /// Mode 2 (OAM Search)
  PPU_MODE_DRAW   = 3  /// Mode 3 (Pixel Transfer)
} PpuMode; 


// - - - FIFO & Fetcher Types - - -

typedef enum 
{
  FETCH_GET_TILE,
  FETCH_GET_TILE_LOW,
  FETCH_GET_TILE_HIGH,
  FETCH_SLEEP,
  FETCH_PUSH
} FetcherState;

typedef struct 
{
  u8    pixel;      /// 2-bit color (0-3)
  u8    palette;    /// Palette index (DMG: 0-1, CGB: 0-7)
  u8    priority;   /// Sprite priority (OAM index in CGB)
  bool  bgPriority; /// BG priority bit
} PpuPixel;

typedef struct 
{
  PpuPixel  pixels[16]; /// Double-buffered FIFO
  u8        head;
  u8        tail;
  u8        size;
} PpuFifo;

typedef struct 
{
  FetcherState  state;
  u8            tileIndex;
  u8            tileAttr;   /// CGB Tile Attributes
  u8            dataLow;
  u8            dataHigh;
  u8            mapAddr;    /// Current map index address
  u8            xOffset;    /// Horizontal tile offset
} PpuFetcher;


/// @brief Main PPU Context
typedef struct 
{
  // - - - Standard I/O Registers (0xFF40 - 0xFF4B)
  u8 lcdc; /// LCD Control 
  u8 stat; /// LCD Status
  u8 scy;  /// Scroll Y
  u8 scx;  /// Scroll X
  u8 ly;   /// Current Scanline
  u8 lyc;  /// LY Compare
  u8 bgp;  /// DMG BG Palette
  u8 obp0; /// DMG Sprite Palette 0
  u8 obp1; /// DMG Sprite Palette 1
  u8 wy;   /// Window Y
  u8 wx;   /// Window X

  // - - - CGB Specific Memory & Registers
  u8 vram[2][0x2000]; /// 16KB Banked VRAM (Bank 0 & 1)
  u8 vramBank;        /// VBK Register (0xFF4F)
  
  u8   bgColorRam[64]; /// Background Color RAM (32 colors)
  u8   bgPaletteIndex; /// BCPS (0xFF68)
  bool bgPaletteAuto;  /// BCPS Bit 7

  u8   objColorRam[64]; /// Object Color RAM (32 colors)
  u8   objPaletteIndex; /// OCPS (0xFF6A)
  bool objPaletteAuto;  /// OCPS Bit 7

  // - - - Internal State & Timing 
  u32 dotClock;         /// T-cycles elapsed in current line (0-455)
  u32 frameClock;       /// Total T-cycles in current frame
  
  PpuFetcher fetcher;
  PpuFifo    bgFifo;
  PpuFifo    objFifo;
  
  u8   pixelsPushed;    /// Pushed to current scanline (0-159)
  u8   scrollXFifoAdj;  /// Internal SCX alignment for FIFO
  bool windowTriggered; /// Did the window activate this line?

  // - - - Hardware Buffers
  u8  oam[0xA0];                                 /// Object Attribute Memory (40 sprites)
  u32 frameBuffer[SCREEN_WIDTH * SCREEN_HEIGHT]; /// RGB32 output
                                                 
  u8  windowLineCounter; /// Counts lines drawn in current window (resets on new window trigger)
} PpuContext;


// - - - Public API - - -

/// @brief Initialize PPU state to post-boot defaults.
void ppuInit(void);

/** 
 * @brief Step the PPU by exactly ONE T-cycle. 
 * Must be called 4 times for every CPU M-cycle.
*/
void ppuStepTCycle(void);

void ppuStepMCycle(void);

/**
 * @brief Read from PPU registers or VRAM/OAM based on current mode. 
 * @param ADDRESS The memory address being accessed (0xFF40-0xFF4B for registers, 0x8000-0x9FFF for VRAM, 0xFE00-0xFE9F for OAM).
 * @return The value read from the specified address, or 0xFF if the access
*/
u8 ppuRead(u16 ADDRESS);

/**
 * @brief Read a byte from VRAM, respecting current bank and access restrictions.
 * @param ADDRESS The VRAM address being accessed (0x8000-0x9FFF).
 * @return The value read from VRAM, or 0xFF if the access is
*/
u8 ppuVRAMRead(u16 ADDRESS);

/**
 * @brief Read a byte from OAM, respecting access restrictions based on current PPU mode.
 * @param ADDRESS The OAM address being accessed (0xFE00-0xFE9F).
 * @return The value read from OAM, or 0xFF if the access is
*/
u8 ppuOAMRead(u16 ADDRESS);

/**
 * @brief Write to PPU registers or VRAM/OAM based on current mode. 
 * @param ADDRESS The memory address being accessed (0xFF40-0xFF4B for registers, 0x8000-0x9FFF for VRAM, 0xFE00-0xFE9F for OAM).
 * @param VALUE The value to write to the specified address. 
*/
void ppuWrite(u16 ADDRESS, u8 VALUE);

/**
 * @brief Write a byte to VRAM, respecting current bank and access restrictions.
 * @param ADDRESS The VRAM address being accessed (0x8000-0x9FFF).
 * @param VALUE The value to write to VRAM. Ignored if access is not allowed.
*/
void ppuVRAMWrite(u16 ADDRESS, u8 VALUE);

/**
 * @brief Write a byte to OAM, respecting access restrictions based on current PPU mode.
 * @param ADDRESS The OAM address being accessed (0xFE00-0xFE9F).
 * @param VALUE The value to write to OAM. Ignored if access is not allowed.
*/
void ppuOAMWrite(u16 ADDRESS, u8 VALUE);

/// @brief Get the current internal PPU Context.
PpuContext* ppuGetContext(void);

/// @brief Helper to convert CGB 15-bit color to 32-bit ARGB.
u32 ppuColorToRGB32(u16 CGB_COLOR);


