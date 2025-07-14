#include "../include/stack.h"
#include "../include/cpu.h"
#include "../include/bus.h"

void stackPush(u8 DATA)
{
  RegisterFile* registers = cpuGetRegisters();
  registers->stackPointer--;
  busWrite(registers->stackPointer, DATA);
}

void stackPush16(u16 DATA)
{
  stackPush((DATA >> 8) & 0xFF);
  stackPush(DATA & 0xFF);
}

u8 stackPop()
{
  RegisterFile* registers = cpuGetRegisters();
  return busRead(registers->stackPointer++);
}

u16 stackPop16()
{
  u16 lo = stackPop();
  u16 hi = stackPop();

  return (hi << 8) | lo;
}
