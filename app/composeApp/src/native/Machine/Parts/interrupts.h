#pragma once
#include "../Utils/bit.h"
#include "cpu.h"
//  - - - Implementing Bus for interaction of machine and buttons
#ifdef __cplusplus
extern "C"{
#endif
    typedef enum
    {
        IT_VBLANK = 1,
        IT_LCD_STAT = 2,
        IT_TIMER = 4,
        IT_SERIAL = 8,
        IT_JOYPAD = 16
    } interrupt_type;

    void cpuRequestInterrupt(interrupt_type t);
    void cpuHandleInterrupts(CPUContext* ctx);
#ifdef __cplusplus
};
#endif
