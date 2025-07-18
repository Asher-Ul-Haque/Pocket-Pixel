#pragma once
#include "../../../defines.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

FORGE_API void stackPush(u8 DATA);
FORGE_API void stackPush16(u16 DATA);

FORGE_API u8  stackPop();
FORGE_API u16 stackPop16();

#ifdef __cplusplus
}
#endif
