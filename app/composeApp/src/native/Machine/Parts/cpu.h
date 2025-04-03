#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - What the CPU does - - -

// - - - start the cpu
FORGE_API void cpuInit();

// - - - tick the clock and update the CPU
FORGE_API void cpuTick();

#ifdef __cplusplus
}
#endif
