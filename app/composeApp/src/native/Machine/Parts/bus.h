#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - read the value from bus 
FORGE_API u8    busRead(u16 ADDRESS);

// - - - to get the context anywhere
FORGE_API void  busWrite(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
