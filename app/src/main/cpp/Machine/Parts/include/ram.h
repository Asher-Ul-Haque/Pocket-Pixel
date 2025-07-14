#pragma once
#include "../../../defines.h"
#include "../../../ForgeLibrary/include/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
  u8 wram[0x2000];
  u8 hram[0x80];
} RAMctx;

FORGE_API void loadRam(u8* BINARY);
FORGE_API u8*  getRam();
FORGE_API u8   wramRead (u16 ADDRESS);
FORGE_API void wramWrite(u16 ADDRESS, u8 VALUE);
FORGE_API u8   hramRead (u16 ADDRESS);
FORGE_API void hramWrite(u16 ADDRESS, u8 VALUE);


#ifdef __cplusplus
}
#endif
