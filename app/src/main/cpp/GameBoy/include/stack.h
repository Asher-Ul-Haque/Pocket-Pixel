#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

// - - - 8 bit operations
void stackPush  (u8 DATA);
u8   stackPop   ();

// - - - 16 bit operations
void stackPush16(u16 DATA);
u16  stackPop16 ();

#ifdef __cplusplus
}
#endif
