#include <ppu/ppu.h>
#include <ppu/ppuRegisters.h>
#include <ppu/internal.h>
#include <common.h>

/**
 * @file oam.c
 * @brief Sprite selection and OAM search (Mode 2) logic
*/


/// @brief Internal structure to hold sprites found during OAM search
typedef struct 
{
  u8 yPos;       /// Sprite Y Position (OAM Byte 0)
  u8 xPos;       /// Sprite X Position (OAM Byte 1)
  u8 tileIndex;  /// Tile Index (OAM Byte 2)
  u8 flags;      /// Attributes/Flags (OAM Byte 3)
  u8 oamIndex;   /// Original OAM Index (0-39) for priority sorting in CGB mode
} SpriteEntry;

typedef struct OamContext
{
  SpriteEntry scanLineSrpites[MAX_SPRITES_SCANLINE];
  u8          spriteCount;
} OamContext;

OamContext* oamGetContext(void);

void ppuOamSearchTick(void);

void ppuOamResetSearch(void);

void ppuOverlayDelaySprites(void);
