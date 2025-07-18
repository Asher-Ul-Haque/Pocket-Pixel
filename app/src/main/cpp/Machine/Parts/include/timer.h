#pragma once
#include "../../../defines.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct timerContext
{
  u16 div;
  u8  tima;
  u8  tma;
  u8  tac;
} TimerContext;

FORGE_API void timerInit();
FORGE_API void timerTick();

FORGE_API void timerWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   timerRead (u16 ADDRESS);

FORGE_API TimerContext* timerGetContext();

#ifdef __cplusplus
}
#endif
