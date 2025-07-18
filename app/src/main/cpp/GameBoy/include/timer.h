#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  u16 div;
  u8  tima;
  u8  tma;
  u8  tac;
} TimerContext;

void timerInit();
void timerTick();

void timerWrite(u16 ADDRESS, u8 VALUE);
u8   timerRead (u16 ADDRESS);

TimerContext* timerGetContext();

#ifdef __cplusplus
}
#endif
