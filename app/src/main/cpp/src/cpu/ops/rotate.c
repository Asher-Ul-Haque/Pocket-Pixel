#include <cpu/instruction.h>
#include <cpu/ops.h>
#include <cpu/registers.h>
#include <cpu/cpu.h>
#include <bus.h>

ExecStatus instrRlca(void)
{
  CpuContext* ctx         = cpuGetContext();
  u8          accumulator = ctx->registers.a;
  u8          bit7        = BIT(accumulator, 7);

  // - - - Shift left and wrap bit 7 around to bit 0 
  ctx->registers.a = (accumulator << 1) | bit7;

  // - - - Flags: Z, N, H are always 0. C is the old bit 7 
  ctx->registers.f = 0;
  if (bit7) ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrRrca(void) 
{
  CpuContext* ctx         = cpuGetContext();
  u8          accumulator = ctx->registers.a;
  u8          bit0        = BIT(accumulator, 0);

  // - - - Shift right and wrap bit 0 around to bit 7
  ctx->registers.a = (accumulator >> 1) | (bit0 << 7);

  // - - - Flags: Z, N, H are always 0. C is the old bit 0.
  ctx->registers.f = 0;
  if (bit0) ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrRla(void)
{
  CpuContext* ctx         = cpuGetContext();
  u8          accumulator = ctx->registers.a;
  u8          oldCarry    = (ctx->registers.f & FLAG_C) ? 1: 0;
  u8          bit7        = BIT(accumulator, 7);

  // - - - shift left, bit 0 becomes the old carry 
  ctx->registers.a = (accumulator << 1) | oldCarry;

  // - - - Flags: Z, N, H always 0. C is the old bit 7 
  ctx->registers.f = 0;
  if (bit7) ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrRra(void)
{
  CpuContext* ctx         = cpuGetContext();
  u8          accumulator = ctx->registers.a;
  u8          oldCarry    = (ctx->registers.f & FLAG_C) ? 1 : 0;
  u8          bit0        = BIT(accumulator, 0);

  // - - - shift right, bit 7 becomes the old carry 
  ctx->registers.a = (accumulator >> 1) | (oldCarry << 7);

  // - - - Flags: Z, N, H always 0. C is the old bit 0 
  ctx->registers.f = 0;
  if (bit0) ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrCbRlcReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val  = cpuGetReg8(instr->reg1);
    u8 bit7 = BIT(val, 7);

    // - - - shift left and wrap bit 7 to bit 0 
    u8 result = (val << 1) | bit7;

    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z=depends, N=0, H=0, C=bit7 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit7)         ctx->registers.f |= FLAG_C; 

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbRlcHL(void)
{
  CpuContext* ctx = cpuGetContext();
  u16 addr = cpuGetReg16(RT_HL);

  // - - - M2: Read data from memory 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Rotate data and write back to memory 
  if (ctx->mCycle == M4)
  {
    u8 data = ctx->latchedVal8;
    u8 b7 = BIT(data, 7);
    u8 result = (data << 1) | b7;

    busWrite(addr, result);

    // - - - Flags: Z = (result == 0), N = 0, H =0, C = b7 
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;
    if (b7) ctx->registers.f          |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbRrcReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val  = cpuGetReg8(instr->reg1);
    u8 bit0 = BIT(val, 0);

    u8 result = (val >> 1) | (bit0 << 7);
    cpuSetReg8(instr->reg1, result);

    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit0)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbRrcHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - - - M3: Read data from emmory into internal latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform RRC logic and write back to the register 
  if (ctx->mCycle == M4)
  {
    u8 data   = ctx->latchedVal8;
    u8 b0     = BIT(data, 0);
    u8 result = (data >> 1) | (b0 << 7);

    busWrite(addr, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C = old bit 0 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (b0)           ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbRlReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val      = cpuGetReg8(instr->reg1);
    u8 oldCarry = (ctx->registers.f & FLAG_C) ? 1 : 0;
    u8 bit7     = BIT(val, 7);

    // - - - shift left and bring the old carry into bit 0 
    u8 result = (val << 1) | oldCarry;

    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z = dynamic, N = 0, H = 0, C = old bit 7 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit7)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbRlHL(void) 
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - - - M3: Read data from memory into internal latch
  if (ctx->mCycle == M2) 
  { 
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE; 
  }

  // - - - M4: Perform RL logic and write result back to memory
  if (ctx->mCycle == M4)
  {
    u8 data     = ctx->latchedVal8;
    u8 oldCarry = (ctx->registers.f & FLAG_C) ? 1 : 0;
    u8 b7       = BIT(data, 7); 
    
    u8 result = (data << 1) | oldCarry;

    busWrite(addr, result);

    // - - - Flags: Z based on result, N=0, H=0, C=old bit 7
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;
    if (b7)          ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE; 
}

ExecStatus instrCbRrReg(void) 
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val      = cpuGetReg8(instr->reg1);
    u8 oldCarry = (ctx->registers.f & FLAG_C) ? 1 : 0;
    u8 bit0     = BIT(val, 0);

    // - - - Shift right and bring the old carry into bit 7
    u8 result = (val >> 1) | (oldCarry << 7);

    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z=dynamic, N=0, H=0, C=old bit 0
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;
    if (bit0)        ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE; 
}

ExecStatus instrCbRrHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - --  M3: Read data from memory into internal latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform RR logic and write reuslt back to memory 
  if (ctx->mCycle == M4)
  {
    u8 data       = ctx->latchedVal8;
    u8 oldCarry   = (ctx->registers.f & FLAG_C) ? 1 : 0;
    u8 bit0       = BIT(data, 0);

    u8 result = (data >> 1) | (oldCarry << 7);

    busWrite(addr, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C = OLD BIT 0 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit0)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
