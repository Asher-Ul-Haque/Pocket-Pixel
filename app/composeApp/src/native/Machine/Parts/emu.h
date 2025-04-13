#pragma once
#include "../Utils/bit.h"
#include "../../ForgeLib/include/logger.h"
#include "../../defines.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - METHODS AND STRUCTS TO CHECK EMU STATE
typedef struct
{
    bool paused;
    bool die;
    bool running;
    u64 ticks;
}emuContext;

int emuRunning(u8* rom, u64 size);
emuContext *emuGetContext();
void emuCycles(int cpuCycles);

#ifdef __cplusplus
}
#endif
