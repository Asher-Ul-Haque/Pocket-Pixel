#pragma once
#include "../Utils/bit.h"
//  - - - Implementing Bus for interaction of machine and buttons
#ifdef __cplusplus
extern "C"{
#endif

u8 bus_read(u16 address);
void bus_write(u16 address, u8 value);

#ifdef __cplusplus
};
#endif
