#include "../include/stack.h"
#include "../include/bus.h"
#include "../include/cpu.h"
/*
    STACK

    SP=0xDFFF

    MEMORY:
    0xDFF7: 00
    0xDFF8: 00
    0xDFF9: 00
    0xDFFA: 00
    0xDFFB: 00
    0xDFFC: 00
    0xDFFD: 00
    0xDFFE: 00
    0xDFFF: 00 <- SP

    PUSH 0x55

    SP-- = 0xDFFE
    MEMORY[0xDFFE] = 0x55

    MEMORY:
    0xDFF7: 00
    0xDFF8: 00
    0xDFF9: 00
    0xDFFA: 00
    0xDFFB: 00
    0xDFFC: 00
    0xDFFD: 00
    0xDFFE: 55 <- SP
    0xDFFF: 00

    PUSH 0x77

    SP-- = 0xDFFD
    MEMORY[0xDFFD] = 0x77

    MEMORY:
    0xDFF7: 00
    0xDFF8: 00
    0xDFF9: 00
    0xDFFA: 00
    0xDFFB: 00
    0xDFFC: 00
    0xDFFD: 77 <- SP
    0xDFFE: 55
    0xDFFF: 00

    val = POP

    val = MEMORY[0xDFFD] = 0x77
    SP++ = 0xDFFE

    MEMORY:
    0xDFF7: 00
    0xDFF8: 00
    0xDFF9: 00
    0xDFFA: 00
    0xDFFB: 00
    0xDFFC: 00
    0xDFFD: 77 
    0xDFFE: 55 <- SP
    0xDFFF: 00


    PUSH 0x88

    SP-- = 0xDFFD
    MEMORY[0xDFFD] = 0x88

    MEMORY:
    0xDFF7: 00
    0xDFF8: 00
    0xDFF9: 00
    0xDFFA: 00
    0xDFFB: 00
    0xDFFC: 00
    0xDFFD: 88 <- SP
    0xDFFE: 55 
    0xDFFF: 00
*/

void stackPush(u8 DATA) 
{
  cpuGetRegisters()->stackPointer--;
  busWrite(cpuGetRegisters()->stackPointer, DATA);
}

void stackPush16(u16 DATA) 
{
  stackPush((DATA >> 8) & 0xFF);
  stackPush(DATA & 0xFF);
}

u8 stackPop() 
{ return busRead(cpuGetRegisters()->stackPointer++); }

u16 stackPop16() 
{
  u16 lo = stackPop();
  u16 hi = stackPop();

  return (hi << 8) | lo;
}
