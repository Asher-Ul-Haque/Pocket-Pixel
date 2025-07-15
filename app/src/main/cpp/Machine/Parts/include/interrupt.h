#pragma once
#include "../../../defines.h"
#include "cpu.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
  INTRPT_VBLANK     = 1,
  INTRPT_LCD_STAT   = 2,
  INTRPT_TIMER      = 4,
  INTRPT_SERIAL     = 8,
  INTRPT_JOYPAD     = 16
} InterruptType;

void cpuRequestInterrupt(InterruptType TYPE);
void cpuHandleInterrupts(CPUContext* CTX);

#ifdef __cplusplus
}
#endif
