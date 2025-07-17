#pragma once
#include "../../../defines.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


FORGE_API u8   IOread(u16 ADDRESS);
FORGE_API void IOwrite(u16 ADDRESS, u8 VALUE);

#ifdef __cplusplus
}
#endif
