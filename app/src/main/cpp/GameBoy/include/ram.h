#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

u8   wramRead (u16 ADDRESS);
void wramWrite(u16 ADDRESS, u8 VALUE);

u8   hramRead (u16 ADDRESS);
void hramWrite(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
