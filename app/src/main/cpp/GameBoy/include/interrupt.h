#pragma once
#include "../../defines.h"
#include "cpu.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - What an interrupt is 
typedef enum 
{
  IT_VBLANK   = 1,
  IT_LCD_STAT = 2,
  IT_TIMER    = 4,
  IT_SERIAL   = 8,
  IT_JOYPAD   = 16
} InterruptType;


// - - - Interrupt functions
void cpuRequestInterrupt(InterruptType INTRPT);
void cpuHandleInterrupts(CPUcontext* CTX);

#ifdef __cplusplus
}
#endif
