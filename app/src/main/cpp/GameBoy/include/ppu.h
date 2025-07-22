#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - Constants - - - 
static const u16 LINES_PER_FRAME = 154;
static const u16 TICKS_PER_LINE  = 456;
static const u16 YRES            = 144;
static const u16 XRES            = 160;


// - - - Structs and Enums - - - 

// - - - current state of fetching 
typedef enum 
{
  FS_TILE,   // - - - current state of the tile fetching 
  FS_DATA0,  // - - - fetching the data byte 1 
  FS_DATA1,  // - - - fetching the data byte 2 
  FS_IDLE,   // - - - doing nothing
  FS_PUSH    // - - - actually push to the framebuffer 
} FetchState;

typedef struct FifoEntry
{
  struct FifoEntry* next;
  u32               color;
} FifoEntry;

typedef struct 
{
  FifoEntry* head;
  FifoEntry* tail;
  u32        size;
} Fifo;

typedef struct 
{
  FetchState curFetchState;
  Fifo       pixelFifo;
  u8         lineX;
  u8         pushedX;
  u8         fetchX;
  u8         bgFetchData[3];
  u8         fetchEntryData[6]; 
  u8         mapY;
  u8         mapX;
  u8         tileY;
  u8         fifoX;
} PixelFifoContext;

typedef struct 
{
  u8 y;
  u8 x;
  u8 tile;
  
  u8 flagCGBpalleteNo : 3;
  u8 flagCGBvrankBank : 1;
  u8 flagPalleteNo    : 1;
  u8 flagXflip        : 1;
  u8 flagYflip        : 1;
  u8 flagBGpallete    : 1;
} OAMentry;

/*
 Bit7   BG and Window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
 Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
 Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
 Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
 Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
 Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)
 */

typedef struct oamLineEntry 
{
  OAMentry             entry;
  struct oamLineEntry* next;
} OAMlineEntry;

typedef struct 
{
  OAMentry  oamRam[40];
  u8        vram[0x2000];

  PixelFifoContext pfc;

  u8            lineSpriteCount;    // - - - 0 - 10 sprites
  OAMlineEntry* lineSprites;        // - - - linked list of current sprites on line.
  OAMlineEntry  lineEntryArray[10]; // - - - memory to use for list.

  u8       fetchedEntryCount;
  OAMentry fetchedEntries[3];       // - - - entries fetched during pipeline.

  u32  currentFrame;
  u32  lineTicks;
  u32* frameBuffer;
} PPUcontext;

FORGE_API void ppuInit();
FORGE_API void ppuTick();

FORGE_API void ppuOAMwrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   ppuOAMread (u16 ADDRESS);

FORGE_API void ppuVramWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   ppuVramRead (u16 ADDRESS);

FORGE_API PPUcontext* ppuGetContext();


#ifdef __cplusplus
}
#endif
