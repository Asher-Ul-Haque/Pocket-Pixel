#pragma once 
#include <common.h>
#include <ppu/internal.h>

/// @brief PPU state machine mode 
typedef enum 
{
  PPU_MODE_HBLANK   = 0,
  PPU_MODE_VBLANK   = 1,
  PPU_MODE_OAM_SCAN = 2,
  PPU_MODE_DRAWING  = 3
} PpuMode;


/**
 * @brief The raw frame buffer output by the PPU mixer.
 * - In DMG Mode: Values are strictly 0, 1, 2, or 3 (the resolved hardware shade).
 * - In CGB Mode: Values are the raw 15-bit RGB555 colors pulled from CRAM.
*/
typedef struct PpuFrame 
{
  u16 pixels[HEIGHT][WIDTH];
} PpuFrame;

typedef struct PpuRegisters 
{
  u8 lcdc;   ///< 0xFF40: LCD Control
  u8 stat;   ///< 0xFF41: LCD Status
  u8 scy;    ///< 0xFF42: Viewport Y
  u8 scx;    ///< 0xFF43: Viewport X
  u8 ly;     ///< 0xFF44: LCD Y Coordinate (Read-Only)
  u8 lyc;    ///< 0xFF45: LY Compare
  u8 dma;    ///< 0xFF46: OAM DMA Source Address & Start
  u8 bgp;    ///< 0xFF47: DMG BG Palette Data
  u8 obp0;   ///< 0xFF48: DMG OBJ Palette 0 Data
  u8 obp1;   ///< 0xFF49: DMG OBJ Palette 1 Data
  u8 wy;     ///< 0xFF4A: Window Y Position
  u8 wx;     ///< 0xFF4B: Window X Position
  u8 key1;   ///< 0xFF4D: CGB Speed Switch Register
  u8 vbk;    ///< 0xFF4F: CGB VRAM Bank Select
  u8 hdma1;  ///< 0xFF51: CGB HDMA Source High
  u8 hdma2;  ///< 0xFF52: CGB HDMA Source Low
  u8 hdma3;  ///< 0xFF53: CGB HDMA Destination High
  u8 hdma4;  ///< 0xFF54: CGB HDMA Destination Low
  u8 hdma5;  ///< 0xFF55: CGB HDMA Length/Mode/Start
  u8 bgpi;   ///< 0xFF68: CGB Background Palette Index
  u8 obpi;   ///< 0xFF6A: CGB Object Palette Index
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
  REG_KEY_1             = 0xFF4D,
  REG_VRAM_BANK         = 0xFF4F,
  REG_HDMA1             = 0xFF51,
  REG_HDMA2             = 0xFF52,
  REG_HDMA3             = 0xFF53,
  REG_HDMA4             = 0xFF54,
  REG_HDMA5             = 0xFF55,
  REG_BG_PALETTE_INDEX  = 0xFF68,
  REG_BG_PALETTE_DATA   = 0xFF69,
  REG_OBJ_PALETTE_INDEX = 0xFF6A,
  REG_OBJ_PALETTE_DATA  = 0xFF6B,
} PpuRegisterAddr;

typedef struct
{
  bool active;  ///< True if an incremental transfer loop is processing bytes
  u16  source;  ///< Pre-calculated absolute source base memory block address
  u8   index;   ///< Active processing byte index tracker (0 to 159)
} PpuOamDma;

typedef struct 
{
  bool active;        ///< Armed flag for active Horizontal Blank multi-line transfers
  u16  source;        ///< Current shifting system memory source pointer
  u16  destination;   ///< Current active destination VRAM address pointer
  u8   blocksLeft;    ///< Value tracker representing blocks remaining for register reads
} PpuCgbDma;

typedef struct 
{
  u8 spriteIndices[SPRITE_MAX_PER_SCANLINE]; ///< Sized to match SPRITE_MAX_PER_SCANLINE
  u8 spriteCount;                            ///< Total matching items cached (0 to 10)
} PpuOamLineBuffer;

typedef enum 
{
  FETCH_STATE_GET_TILE_MAP    = 0,
  FETCH_STATE_GET_TILE_ATTR   = 1,
  FETCH_STATE_GET_TILE_DATA_L = 2,
  FETCH_STATE_GET_TILE_DATA_H = 3,
  FETCH_STATE_PUSH_TO_FIFO    = 4
} PpuFetcherState;

typedef struct 
{
  PpuFetcherState state;
  u8              stepClock;       ///< Tracks the internal 2-dot timing subdivisions
  u8              tileMapIndex;    ///< Cached tile VRAM index byte
  u8              tileAttributes;  ///< Cached CGB bank-1 attribute byte
  u8              tileDataLow;     ///< Low bit row data byte
  u8              tileDataHigh;    ///< High bit row data byte
  u8              fetcherX;        ///< Horizontal tile rendering cursor index
} PpuPixelFetcher;

typedef struct 
{
  u8   colorIndex;      ///< Interleaved 2-bit color index (0-3)
  u8   palette;         ///< BG/Win: CGB 0-7 | OBJ: DMG 0-1, CGB 0-7
  bool spritePriority;  ///< True if OBJ priority bit demands rendering behind BG 1-3
  bool bgPriority;      ///< True if CGB BG attribute priority bit is set
} PpuPixel;

typedef struct 
{
  PpuPixel pixels[FIFO_CAPACITY];  
  u8       count;   ///< Current population load
  u8       head;    ///< Read cursor (Pop)
  u8       tail;    ///< Write cursor (Push)
} PpuFifo;


typedef struct PpuContext 
{
  // - - - Internal Memory 
  u8 vram[VRAM_BANK_COUNT][VRAM_BANK_SIZE];
  u8 oam [OAM_SIZE];

  PpuRegisters      registers;
  PpuOamDma         oamDma;
  PpuCgbDma         cgbDma;
  PpuOamLineBuffer  scanlineOamBuffer;
  PpuPixelFetcher   fetcher;

  PpuFifo  bgFifo;
  PpuFifo  objFifo;
  u8       screenX;         ///< Absolute horizontal coordinate pushed to display
  u8       droppedPixels;   ///< Track SCX % 8 fine scroll discards
  bool     spriteFetching;  ///< Lockout flag when injecting objects

  // - - - CGB Palette Ram 
  u8 bgPaletteRam [PALETTE_RAM_SIZE];
  u8 objPaletteRam[PALETTE_RAM_SIZE];

  PpuFrame frameBuffer; ///< 160x144 matrix for platform 

  // - - - State Machine
  PpuMode mode;               ///< Active operational mode (0 to 3)
  u32     dotCount;           ///< Horizontal dot clock counter (0 to 455)
  u16     mode3Duration;      ///< Calculated elastic length for current line (172 to 289 dots) 
  bool    frameReady;         ///< Flag signaling completed vertical sweep 
  bool    statLineState;      ///< Interrupt blocking single-wire logic gate tracker
  bool    windowTriggered;    ///< Screen row visibility flag 
  u8      windowLineCounter;  ///< Internal hidden vertical window row tracker
} PpuContext;


// - - - Ppu IO - - - 

/// @brief: Vram memory area (0x8000 - 0x9FFF)
u8   ppuReadVram (u16 ADDR);
void ppuWriteVram(u16 ADDR, u8 VALUE);

/// @brief: oam MEMORY AREA (0xFE00 - 0xFE9F)
u8   ppuReadOam (u16 ADDR);
void ppuWriteOam(u16 ADDR, u8 VALUE);

/// @brief Core standard & CGB IO register (0xFF40 - 0xFF55)
u8   ppuReadIo (u16 ADDR);
void ppuWriteIo(u16 ADDR, u8 VALUE);

/// @brief: Color palette memory access ports (0xFF68 - 0xFF6B)
u8   ppuReadCram (u16 ADDR);
void ppuWriteCram(u16 ADDR, u8 VALUE);


// - - - Main functions - - - 

void        ppuInit(void);
void        ppuTick(void);
PpuContext* ppuGetContext(void);


// - - - DMA - - -

void ppuDmaTrigger (u8 VALUE);
void ppuHdmaTrigger(u8 VALUE);

// - - - Interrupt - - - 

void ppuUpdateInterrupts(void);
void ppuExecuteSpeedSwitch(void);

// - - - Timing - - - 

void ppuExecuteOamScan(void);
u16  ppuGetSpriteTimingPenalties(void);


// - - - Oam clock linked operations - - - 

void ppuStepOamDma(void);
void ppuCheckHblankDma(void);


// - - - Pixel Fetcher - - - 

void ppuResetFetcher(void);
void ppuStepPixelFetcher(void);
bool ppuPushTileToFifo(void);


// - - - Fifo functions - - - 

void ppuResetFifos(void);
bool ppuPushTileToFifo(void);
void ppuStepPixelMixer(void);
void ppuInjectSpriteToFifo(u8 SPRITE_LINE_BUFFER_INDEX);
void ppuPushPixelToScreen(u8 SCREEN_X, u8 SCREEN_Y, u16 RGB_555);
