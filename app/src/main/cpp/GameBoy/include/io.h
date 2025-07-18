#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

u8   ioRead (u16 ADDRESS);
void ioWrite(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
