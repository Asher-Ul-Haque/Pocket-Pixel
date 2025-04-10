#pragma once
#include "../Utils/bit.h"
//  - - - Implementing Bus for interaction of machine and buttons
#ifdef __cplusplus
extern "C"{
#endif

u8 busRead(u16 address);
    u16 busRead16(u16 address);
void busWrite(u16 address, u8 value);
void busWrite16(u16 address, u16 value);

#ifdef __cplusplus
};
#endif
