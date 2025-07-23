#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - STRUCTS AND CONTEXTS - - - 

// - - - lcd state
typedef struct 
{
  // - - - registers
  u8 control;
  u8 status;
  u8 scrollY;
  u8 scrollX;
  u8 ly;
  u8 lyCompare;
  u8 dma;
  u8 bgPalette;
  u8 objPalette[2];
  u8 windowY;
  u8 windowX;

  // - - - other data
  u32 bgColors[4];
  u32 sp1Colors[4];
  u32 sp2Colors[4];
} LCDcontext;

// - - - LCD modes
typedef enum 
{
  MODE_HBLANK,
  MODE_VBLANK,
  MODE_OAM,
  MODE_XFER
} LCDmode;

// - - - LCD STAT sources
typedef enum 
{
  SS_HBLANK = (1 << 3),
  SS_VBLANK = (1 << 4),
  SS_OAM    = (1 << 5),
  SS_LYC    = (1 << 6),
} StatSrc;


// - - - MACROS AND DEFINES - - -

// - - - LCD control bits
#define LCD_CNTRL_BGW_ENABLE    (BIT(lcdGetContext()->control, 0))
#define LCD_CNTRL_OBJ_ENABLE    (BIT(lcdGetContext()->control, 1))
#define LCD_CNTRL_OBJ_HEIGHT    (BIT(lcdGetContext()->control, 2) ? 16 : 8)
#define LCD_CNTRL_BG_MAP_AREA   (BIT(lcdGetContext()->control, 3) ? 0x9C00 : 0x9800)
#define LCD_CNTRL_BGW_DATA_AREA (BIT(lcdGetContext()->control, 4) ? 0x8000 : 0x8800)
#define LCD_CNTRL_WIN_ENABLE    (BIT(lcdGetContext()->control, 5))
#define LCD_CNTRL_WIN_MAP_AREA  (BIT(lcdGetContext()->control, 6) ? 0x9C00 : 0x9800)
#define LCD_CNTRL_LCD_ENABLE    (BIT(lcdGetContext()->control, 7))

// - - - LCD status bits
#define LCD_STAT_MODE           ((LCDmode)(lcdGetContext()->status & 0b11))
#define LCD_STAT_MODE_SET(mode) { lcdGetContext()->status &= ~0b11; lcdGetContext()->status |= mode; }
#define LCD_STAT_LYC            (BIT(lcdGetContext()->status, 2))
#define LCD_STAT_LYC_SET(b)     (BIT_SET(lcdGetContext()->status, 2, b))
#define LCD_STAT_STAT_INT(src)  (lcdGetContext()->status & src)


// - - - FUNCTIONS - - -

FORGE_API LCDcontext* lcdGetContext();
FORGE_API void        lcdInit();
FORGE_API u8          lcdRead(u16 ADDRESS);
FORGE_API void        lcdWrite(u16 ADDRESS, u8 VALUE);

#ifdef __cplusplus
}
#endif
