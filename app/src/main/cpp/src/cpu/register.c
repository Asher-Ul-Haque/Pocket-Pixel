#include <cpu/cpu.h>
#include <cpu/registers.h>


// - - - 8 bit helpers - - - 

u8 cpuGetReg8(RegType REGISTER) 
{
  CpuContext* ctx = cpuGetContext();
  switch (REGISTER) 
  {
    case RT_A: return ctx->registers.a;
    case RT_B: return ctx->registers.b;
    case RT_C: return ctx->registers.c;
    case RT_D: return ctx->registers.d;
    case RT_E: return ctx->registers.e;
    case RT_H: return ctx->registers.h;
    case RT_L: return ctx->registers.l;
    default: 
      FORGE_ASSERT_DEBUG(false, "Invalid 8-bit register read");
      return 0;
  }
}

void cpuSetReg8(RegType REG, u8 VAL) 
{
  CpuContext* ctx = cpuGetContext();
  switch (REG) 
  {
    case RT_A: ctx->registers.a = VAL; break;
    case RT_B: ctx->registers.b = VAL; break;
    case RT_C: ctx->registers.c = VAL; break;
    case RT_D: ctx->registers.d = VAL; break;
    case RT_E: ctx->registers.e = VAL; break;
    case RT_H: ctx->registers.h = VAL; break;
    case RT_L: ctx->registers.l = VAL; break;
    default: 
      FORGE_ASSERT_DEBUG(false, "Invalid 8-bit register write");
      break;
  }
}

u16 cpuGetReg16(RegType REG) 
{
  CpuContext* ctx = cpuGetContext();
  switch (REG) 
  {
    case RT_BC: return (ctx->registers.b << 8) | ctx->registers.c;
    case RT_DE: return (ctx->registers.d << 8) | ctx->registers.e;
    case RT_HL: return (ctx->registers.h << 8) | ctx->registers.l;
    case RT_SP: return ctx->registers.stackPointer;
    case RT_AF: return (ctx->registers.a << 8) | ctx->registers.f;
    default: return 0;
  }
}

void cpuSetReg16(RegType REGISTER, u16 VALUE) 
{
  CpuContext* ctx = cpuGetContext();

  switch (REGISTER)
  {
    // - - - F register lower 4 bits are hardwired to 0
    case RT_AF: 
      ctx->registers.a = (u8)(VALUE >> 8); 
      ctx->registers.f = (u8)(VALUE & 0xF0u); 
      return;

    case RT_BC: 
      ctx->registers.b = (u8)(VALUE >> 8); 
      ctx->registers.c = (u8)(VALUE & 0xFFu); 
      return;

    case RT_DE: 
      ctx->registers.d = (u8)(VALUE >> 8); 
      ctx->registers.e = (u8)(VALUE & 0xFFu); 
      return;

    case RT_HL: 
      ctx->registers.h = (u8)(VALUE >> 8); 
      ctx->registers.l = (u8)(VALUE & 0xFFu); 
      return;

    case RT_SP: 
      ctx->registers.stackPointer = VALUE; 
      return;

    case RT_PC: 
      ctx->registers.programCounter = VALUE; 
      return;

    default:
      FORGE_LOG_ERROR("[CPU] writeReg16: invalid RegType=%d", (int)REGISTER);
      ctx->halted = true;
      return;
  }
}
