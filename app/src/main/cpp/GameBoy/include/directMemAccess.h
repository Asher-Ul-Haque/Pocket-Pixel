#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

FORGE_API bool dmaTransferring();
FORGE_API void dmaStart(u8 START);
FORGE_API void dmaTick();





#ifdef __cplusplus
}
#endif
