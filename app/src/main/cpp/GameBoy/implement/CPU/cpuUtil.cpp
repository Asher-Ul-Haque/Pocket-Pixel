#include "../../include/cpu.h"
#include "../../include/bus.h"

extern CPUcontext ctx;

u16 reverse(u16 N) 
{ return ((N & 0xFF00) >> 8) | ((N & 0x00FF) << 8); }

u16 cpuReadRegister(RegType REG) 
{
  switch(REG) 
  {
    case RT_A   : return ctx.regs.accumulator;
    case RT_F   : return ctx.regs.flags;
    case RT_B   : return ctx.regs.b;
    case RT_C   : return ctx.regs.c;
    case RT_D   : return ctx.regs.d;
    case RT_E   : return ctx.regs.e;
    case RT_H   : return ctx.regs.h;
    case RT_L   : return ctx.regs.l;

    case RT_AF  : return reverse(*((u16 *)&ctx.regs.accumulator));
    case RT_BC  : return reverse(*((u16 *)&ctx.regs.b));
    case RT_DE  : return reverse(*((u16 *)&ctx.regs.d));
    case RT_HL  : return reverse(*((u16 *)&ctx.regs.h));

    case RT_PC  : return ctx.regs.programCounter;
    case RT_SP  : return ctx.regs.stackPointer;
    default     : return 0;
  }
}

void cpuSetRegister(RegType REG, u16 VAL) 
{
  switch(REG) 
  {
    case RT_A  : ctx.regs.accumulator   = VAL & 0xFF; break;
    case RT_F  : ctx.regs.flags         = VAL & 0xFF; break;
    case RT_B  : ctx.regs.b             = VAL & 0xFF; break;
    case RT_C  : ctx.regs.c             = VAL & 0xFF; break;
    case RT_D  : ctx.regs.d             = VAL & 0xFF; break;
    case RT_E  : ctx.regs.e             = VAL & 0xFF; break;
    case RT_H  : ctx.regs.h             = VAL & 0xFF; break;
    case RT_L  : ctx.regs.l             = VAL & 0xFF; break;

    case RT_AF : *((u16 *)&ctx.regs.accumulator)    = reverse(VAL); break;
    case RT_BC : *((u16 *)&ctx.regs.b)              = reverse(VAL); break;
    case RT_DE : *((u16 *)&ctx.regs.d)              = reverse(VAL); break;
    case RT_HL : *((u16 *)&ctx.regs.h)              = reverse(VAL); break;

    case RT_PC   : ctx.regs.programCounter    = VAL; break;
    case RT_SP   : ctx.regs.stackPointer      = VAL; break;
    case RT_NONE : break;
  }
}


u8 cpuReadRegister8(RegType REG) 
{
  switch(REG) 
  {
    case RT_A  : return ctx.regs.accumulator;
    case RT_F  : return ctx.regs.flags;
    case RT_B  : return ctx.regs.b;
    case RT_C  : return ctx.regs.c;
    case RT_D  : return ctx.regs.d;
    case RT_E  : return ctx.regs.e;
    case RT_H  : return ctx.regs.h;
    case RT_L  : return ctx.regs.l;
    case RT_HL : return busRead(cpuReadRegister(RT_HL));
    default:
      FORGE_LOG_ERROR("**INVALID REG8: %d", REG);
      FORGE_ASSERT(false);
  }
}

void cpuSetRegister8(RegType REG, u8 VAL) 
{
  switch(REG) 
  {
    case RT_A  : ctx.regs.accumulator   = VAL & 0xFF; break;
    case RT_F  : ctx.regs.flags         = VAL & 0xFF; break;
    case RT_B  : ctx.regs.b             = VAL & 0xFF; break;
    case RT_C  : ctx.regs.c             = VAL & 0xFF; break;
    case RT_D  : ctx.regs.d             = VAL & 0xFF; break;
    case RT_E  : ctx.regs.e             = VAL & 0xFF; break;
    case RT_H  : ctx.regs.h             = VAL & 0xFF; break;
    case RT_L  : ctx.regs.l             = VAL & 0xFF; break;
    case RT_HL : busWrite(cpuReadRegister(RT_HL), VAL); break;
    default:
      FORGE_LOG_ERROR("**INVALID REG8: %d\n", REG);
      FORGE_ASSERT(false);
  }
}

RegisterFile* cpuGetRegisters() 
{ return &ctx.regs; }

u8 cpuGetInterruptFlags() 
{ return ctx.interruptFlags; }

void cpuSetInterruptFlags(u8 VALUE) 
{ ctx.interruptFlags = VALUE; }
