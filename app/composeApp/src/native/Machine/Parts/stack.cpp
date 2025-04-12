#include "stack.h"
#include "cpu.h"
#include "bus.h"

void stackPush(u8 data)
{
    cpuGetRegister()->stackPointer--;
    busWrite(cpuGetRegister()->stackPointer, data);
}

void stackPush16(u16 data)
{
    stackPush((data >> 8)  & 0xFF);
    stackPush(data & 0xFF);
}

u8 stackPop()
{
    return (busRead(cpuGetRegister()->stackPointer++));
}

u16 stackPop16()
{
    u16 lo = stackPop();
    u16 hi = stackPop();

    return (hi << 8) | lo;
}
