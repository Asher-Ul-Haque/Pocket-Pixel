#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

FORGE_API void timerDelay(u32 MILLISECONDS);
FORGE_API void timerInit();
FORGE_API void timerTick();

#ifdef __cplusplus
}
#endif