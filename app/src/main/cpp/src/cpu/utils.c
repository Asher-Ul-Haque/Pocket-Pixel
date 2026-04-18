#include <cpu/cpu.h>
#include <bus.h>


void cpuStackWriteHi(u16 VALUE)
{
  RegisterFile* r = &cpuGetContext()->registers;
  r->stackPointer--;
  busWrite(r->stackPointer, (u8)((VALUE >> 8) & 0xFFu));
}

void cpuStackWriteLo(u16 VALUE)
{
  RegisterFile* r = &cpuGetContext()->registers;
  r->stackPointer--;
  busWrite(r->stackPointer, (u8)(VALUE & 0xFFu));
}

u8 cpuStackReadLo(void)
{
  RegisterFile* r  = &cpuGetContext()->registers;
  u8            lo = busRead(r->stackPointer);
  r->stackPointer++;
  return lo;
}

u8 cpuStackReadHi(void)
{
  RegisterFile* r  = &cpuGetContext()->registers;
  u8            hi = busRead(r->stackPointer);
  r->stackPointer++;
  return hi;
}

bool cpuEvalCond(ConditionType COND)
{
  const CpuContext*   ctx = cpuGetContext();
  const RegisterFile* r   = &ctx->registers;

  switch (COND)
  {
    case CT_NONE: return true;
    case CT_NZ:   return !cpuFlagZ(r);
    case CT_Z:    return  cpuFlagZ(r);
    case CT_NC:   return !cpuFlagC(r);
    case CT_C:    return  cpuFlagC(r);
    default: return false;
  }
}
