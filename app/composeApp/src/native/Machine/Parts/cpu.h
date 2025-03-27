#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - What the CPU does - - - 

// - - - start the cpu 
FORGE_API void CPUinit();

// - - - tick the clock and update the CPU
FORGE_API void CPUtick();

#ifdef __cplusplus
}
#endif
