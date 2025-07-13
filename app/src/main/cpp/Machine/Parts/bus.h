#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"

#ifdef __cplusplus
extern "C" {
#endif


FORGE_API u8    busRead(u16 ADDRESS);
FORGE_API void  busWrite(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
