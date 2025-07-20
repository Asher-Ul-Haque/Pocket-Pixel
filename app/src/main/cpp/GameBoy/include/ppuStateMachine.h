#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

FORGE_API void ppuModeOAM();
FORGE_API void ppuModeXfer();
FORGE_API void ppuModeVblank();
FORGE_API void ppuModeHblank();







#ifdef __cplusplus
}
#endif
