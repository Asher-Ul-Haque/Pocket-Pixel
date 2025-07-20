#include "../include/ppu.h"
#include "../include/ppuStateMachine.h"
#include "../include/lcd.h"
#include "../include/cpu.h"
#include "../include/interrupt.h"
#include <ctime>


void incrementLY()
{
  lcdGetContext()->ly++;
  if (lcdGetContext()->ly == lcdGetContext()->lyCompare)
  {  
    LCD_STAT_LYC_SET(1); 
    if (LCDS_STAT_INT(SS_LYC))
    {
      cpuRequestInterrupt(IT_LCD_STAT);
    }
  }
  else LCD_STAT_LYC_SET(0);
}

void ppuModeOAM()
{
  if (ppuGetContext()->lineTicks >= 80)        LCD_STAT_MODE_SET(MODE_XFER);
}

void ppuModeXfer()
{
  if (ppuGetContext()->lineTicks >= 80 + 172)  LCD_STAT_MODE_SET(MODE_HBLANK);
}


static u32  targetFrameTime = 1000 / 60;
static long prevFrameTime   = 0;
static long startTimer      = 0;
static long frameCount      = 0;

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

void ppuModeHblank()
{
  if (ppuGetContext()->lineTicks >= TICKS_PER_LINE)
  {
    incrementLY();

    if (lcdGetContext()->ly >= Y_RES)
    {
      LCD_STAT_MODE_SET(MODE_VBLANK);
      cpuRequestInterrupt(IT_VBLANK);

      if (LCDS_STAT_INT(SS_VBLANK))
      { cpuRequestInterrupt(IT_LCD_STAT); }

      ppuGetContext()->currentFrame++;

      u64 end       = getTicks();
      u32 frameTime = end - prevFrameTime;

      if (frameTime < targetFrameTime) 
      { delay(targetFrameTime - frameTime); }

      if (end - startTimer >= 1000)
      {
        u32 fps = frameCount;
        startTimer = end;
        frameCount = 0;

        FORGE_LOG_TRACE("fps : %d", fps);
      }

      frameCount++;
      prevFrameTime = getTicks();
    }
    else LCD_STAT_MODE_SET(MODE_OAM);

    ppuGetContext()->lineTicks = 0;
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
      lcdGetContext()->ly = 0;  
    }

    ppuGetContext()->lineTicks = 0;
  }
}

