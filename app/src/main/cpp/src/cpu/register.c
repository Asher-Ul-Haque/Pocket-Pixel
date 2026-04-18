#include <cpu/cpu.h>
#include <cpu/registers.h>

void writeReg8(RegType REGISTER, u8 VALUE)
{
  CpuContext* ctx = cpuGetContext();
  switch (REGISTER)
  {
    case RT_A: ctx->registers.a = VALUE; return;
    case RT_B: ctx->registers.b = VALUE; return;
    case RT_C: ctx->registers.c = VALUE; return;
    case RT_D: ctx->registers.d = VALUE; return;
    case RT_E: ctx->registers.e = VALUE; return;
    case RT_H: ctx->registers.h = VALUE; return;
    case RT_L: ctx->registers.l = VALUE; return;
    default:
      FORGE_LOG_ERROR("[CPU] writeReg8: invalid RegType=%d", (int)REGISTER);
      ctx->halted = true;
      return;
  }
}

void writeReg16(RegType REGISTER, u16 VALUE)
{
  CpuContext* ctx = cpuGetContext();
  switch (REGISTER)
  {
    case RT_AF: ctx->registers.a = (u8)(VALUE >> 8); ctx->registers.f = (u8)(VALUE & 0xF0u); return;
    case RT_BC: ctx->registers.b = (u8)(VALUE >> 8); ctx->registers.c = (u8)(VALUE & 0xFFu); return;
    case RT_DE: ctx->registers.d = (u8)(VALUE >> 8); ctx->registers.e = (u8)(VALUE & 0xFFu); return;
    case RT_HL: ctx->registers.h = (u8)(VALUE >> 8); ctx->registers.l = (u8)(VALUE & 0xFFu); return;
    case RT_SP: ctx->registers.stackPointer   = VALUE; return;
    case RT_PC: ctx->registers.programCounter = VALUE; return;
    default:
      FORGE_LOG_ERROR("[CPU] writeReg16: invalid RegType=%d", (int)REGISTER);
      ctx->halted = true;
      return;
  }
}
