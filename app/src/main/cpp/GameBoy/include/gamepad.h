#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  u8 start   : 1;
  u8 select  : 1;
  u8 a       : 1;
  u8 b       : 1;
  u8 up      : 1;
  u8 down    : 1;
  u8 left    : 1;
  u8 right   : 1;
} GamepadState;

FORGE_API void gamepadInit();
FORGE_API bool gamepadButtonSel();
FORGE_API bool gamepadDirSel();
FORGE_API void gamepadWrite(u8 VALUE);
FORGE_API u8   gamepadRead();

FORGE_API GamepadState* gamepadGetState();
#ifdef __cplusplus
}
#endif
