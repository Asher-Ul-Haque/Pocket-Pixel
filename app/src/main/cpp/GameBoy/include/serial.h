#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - Serial COntext structure 

typedef struct 
{
  u8 SB; // - - - data register    (0xFF01)
  u8 SC; // - - - control register (0xFF02)

  // - - - internal state 
  bool isTransfering;
  u8   recievedNetworkByte;
  bool newNetworkByteAvaiable;
} SerialContext;


// - - - Functions - - - 

FORGE_API void           serialInit();
FORGE_API SerialContext* serialGetContext();
FORGE_API u8             serialRead(u16 ADDRESS);
FORGE_API void           serialWrite(u16 ADDRESS, u8 VALUE);
FORGE_API void           serialReceiveNetworkByte(u8 BYTE);

#ifdef __cplusplus
}
#endif
