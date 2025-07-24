#include "../../include/ppu.h"
#include "../../include/ppuStateMachine.h"
#include "../../include/lcd.h"
#include "../../include/interrupt.h"
#include <string.h>
#include <time.h>

// - - - forward declarations because who is making a header file for 2 functions
void pipelineFifoReset();
void pipelineProcess();
bool windowVisible();


// - - - FPS controls - - - 

static u32 targetFPS      = 120;
static u64 prevFrameTime  = 0;
static u64 startTimer     = 0;
static u64 frameCount     = 0;

static u64 getTicks()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (u64)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void delay(u64 MS)
{
  struct timespec req = 
    {
      .tv_sec  = (time_t)(MS / 1000),
      .tv_nsec = (long)((MS % 1000) * 1000000)
    };
  nanosleep(&req, NULL);
}


void incrementLY() 
{
  LCDcontext* lcdCtx = lcdGetContext();

  if (windowVisible() && 
    lcdCtx->ly >= lcdCtx->windowY &&
    lcdCtx->ly  < lcdCtx->windowY + YRES)
  {  ppuGetContext()->windowLine++; }
  lcdCtx->ly++;

  if (lcdCtx->ly == lcdCtx->lyCompare) 
  {
    LCD_STAT_LYC_SET(1);
    if (LCD_STAT_STAT_INT(SS_LYC)) cpuRequestInterrupt(IT_LCD_STAT);
  } 
  else  LCD_STAT_LYC_SET(0);
}

void loadLineSprites() 
{
  PPUcontext* ppuCtx        = ppuGetContext();
  i32         curY          = lcdGetContext()->ly;
  u8          spriteHeight  = LCD_CNTRL_OBJ_HEIGHT;

  memset(ppuCtx->lineEntryArray, 0, 
    sizeof(ppuCtx->lineEntryArray));

  for (i32 i = 0; i < 40; i++) 
  {
    OAMentry e = ppuCtx->oamRAM[i];

    // - - - invisible
    if (!e.x) continue;

    // - - - max 10 sprites per line
    if (ppuCtx->lineSpriteCount >= 10)  break;

    // - - - sprite is not on the current line
    if (e.y <= curY + 16 && e.y + spriteHeight > curY + 16) 
    {
      OAMlineEntry* entry = &ppuCtx->lineEntryArray[ppuCtx->lineSpriteCount++];
      entry->entry        = e;
      entry->next         = NULL;

      if (!ppuCtx->lineSprites ||
           ppuCtx->lineSprites->entry.x > e.x) 
      {
        entry->next                   = ppuCtx->lineSprites;
        ppuCtx->lineSprites  = entry;
        continue;
      }

      // - - - do some sorting
      OAMlineEntry* le   = ppuCtx->lineSprites;
      OAMlineEntry* prev = le;

      while(le) 
      {
        if (le->entry.x > e.x) 
        {
          prev->next  = entry;
          entry->next = le;
          break;
        }

        if (!le->next) 
        {
          le->next = entry;
          break;
        }

        prev = le;
        le   = le->next;
      }
    }
  }
}

void ppuModeOAM() 
{
  PPUcontext* ppuCtx = ppuGetContext();

  if (ppuCtx->lineTicks >= 80) 
  {
    LCD_STAT_MODE_SET(MODE_XFER);

    ppuCtx->pfc.curFetchState  = FS_TILE;
    ppuCtx->pfc.lineX          = 0;
    ppuCtx->pfc.fetchX         = 0;
    ppuCtx->pfc.pushedX        = 0;
    ppuCtx->pfc.fifoX          = 0;
  }

  // - - - read oam on the first tick only
  if (ppuCtx->lineTicks == 1) 
  {
    ppuCtx->lineSprites     = 0;
    ppuCtx->lineSpriteCount = 0;

    loadLineSprites();
  }
}

void ppuModeXfer() 
{
  pipelineProcess();

  if (ppuGetContext()->pfc.pushedX >= XRES) 
  {
    pipelineFifoReset();

    LCD_STAT_MODE_SET(MODE_HBLANK);

    if (LCD_STAT_STAT_INT(SS_HBLANK)) cpuRequestInterrupt(IT_LCD_STAT);
  }
}

void ppuModeVblank() 
{
  if (ppuGetContext()->lineTicks >= TICKS_PER_LINE) 
  {
    incrementLY();

    if (lcdGetContext()->ly >= LINES_PER_FRAME) 
    {
      LCD_STAT_MODE_SET(MODE_OAM);
      lcdGetContext()->ly         = 0;
      ppuGetContext()->windowLine = 0;
    }

    ppuGetContext()->lineTicks = 0;
  }
}


void ppuModeHblank() 
{
  if (ppuGetContext()->lineTicks >= TICKS_PER_LINE) 
  {
    incrementLY();

    if (lcdGetContext()->ly >= YRES) 
    {
      LCD_STAT_MODE_SET(MODE_VBLANK);

      cpuRequestInterrupt(IT_VBLANK);

      if (LCD_STAT_STAT_INT(SS_VBLANK)) cpuRequestInterrupt(IT_LCD_STAT);

      ppuGetContext()->currentFrame++;

      // - - - calc FPS
      u64 currentTime   = getTicks();
      u64 frameDuration = 1000 / targetFPS;

      // - - - Delay to maintain target FPS
      u64 elapsed = currentTime - prevFrameTime;
      if (elapsed < frameDuration) 
      {
        delay(frameDuration - elapsed);
        currentTime = getTicks();
      }

      frameCount++;

      if (currentTime - startTimer >= 1000) 
      {
        u32 fps    = frameCount;
        startTimer = currentTime;
        frameCount = 0;

        FORGE_LOG_TRACE("FPS: %d", fps);
      }

      prevFrameTime = currentTime;
    } 
    else LCD_STAT_MODE_SET(MODE_OAM);

    ppuGetContext()->lineTicks = 0;
  }
}
