#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

// - - - Emulator struct 
typedef struct 
{
  bool paused;
  bool running;
  bool die;
  u64  ticks;
} EMUcontext;

EMUcontext* emuGetContext();

void emuCycles(i32 CPU_CYCLES);


#ifdef __cplusplus
}
#endif
