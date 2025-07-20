#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

static const i32 LINES_PER_FRAME  = 154;
static const i32 TICKS_PER_LINE   = 456;
static const i32 Y_RES            = 144;
static const i32 X_RES            = 160;

/*
  BIT 7 : BG and Window over OBJ (0 = No, 1 = BG and Window colors 1 - 3 over the OBJ)
  BIT 6 : Y flip  (0 = Normal, 1 = Vertically mirrored)
  BIT 5 : X flip  (0 = Normal, 1 = Horrizontally mirrored)
  BIT 4 : Pallete number **Non CGB Mode only** (0=OBP0), 1 = OBP1)
  BIT 3 : Tile VRAM Bank **CGB Mode Only** (0=Bank 0, 1 =Bank 1)
  BIT 2 - 0 : Pallete number **CGB Mode Only** (0BP0-7)
*/

typedef struct 
{
  u8 y;
  u8 x;
  u8 tile;

  u8 flagGBCpalleteNo : 3;
  u8 flagGBCVramBank  : 1;
  u8 flagPalleteNo    : 1;
  u8 flagXflip        : 1;
  u8 flagYflip        : 1;
  u8 flagBGpriority   : 1;
} OAMentry;


typedef struct 
{
  OAMentry oamRam[40];
  u8       vram[0x2000];

  u32      currentFrame;
  u32      lineTicks;
  u32*     frameBuffer;
} PPUcontext;


FORGE_API void ppuInit(u32* FRAME_BUFFER);
FORGE_API void ppuTick();

FORGE_API void ppuOAMwrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   ppuOAMread(u16 ADDRESS);

FORGE_API void ppuVRAMwrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   ppuVRAMread(u16 ADDRESS);

FORGE_API PPUcontext* ppuGetContext();

#ifdef __cplusplus
}
#endif
