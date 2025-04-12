#pragma once
#include "../Utils/bit.h"
//  - - - Implementing Bus for interaction of machine and buttons
#ifdef __cplusplus
extern "C"{
#endif

void stackPush(u8 data);
    void stackPush16 (u16 data);
    u8 stackPop();
    u16 stackPop16();
#ifdef __cplusplus
};
#endif