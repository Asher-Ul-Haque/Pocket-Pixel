#pragma once
#include "../Utils/bit.h"
//  - - - Implementing Bus for interaction of machine and buttons
#ifdef __cplusplus
extern "C"{
#endif

u8 wramRead(u16 address);
void wramWrite(u16 address, u8 value);

u8 hramRead(u16 address);
void hramWrite(u16 address, u8 value);

#ifdef __cplusplus
};
#endif