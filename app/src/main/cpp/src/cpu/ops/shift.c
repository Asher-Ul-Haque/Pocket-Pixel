#include <bus.h>
#include <cpu/cpu.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>
#include <cpu/ops.h>

ExecStatus instrCbSlaReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val  = cpuGetReg8(instr->reg1);
    u8 bit7 = BIT(val, 7);

    // - - - shift left, bit 0 is filled with 0 in C 
    u8 result = val << 1;

    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z abased on result, N = 0, H = 0, C= old bit 7 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit7)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSlaHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - - - M2: Read data from memory into internal latch 
  if (ctx->mCycle == M2)  return EXEC_STATUS_CONTINUE;

  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform SLA logic and write back into memory 
  if (ctx->mCycle == M4)
  {
    u8 data   = ctx->latchedVal8;
    u8 bit7   = BIT(data, 7);
    u8 result = data << 1;

    busWrite(addr, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C - old bit 7 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit7)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSraReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val  = cpuGetReg8(instr->reg1);
    u8 bit0 = BIT(val, 0);
    u8 bit7 = BIT(val, 7);

    // - - - Shift right and mask / OR to ensure bit 7 stays the same 
    u8 result = (val >> 1) | bit7;

    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C = old bit 0 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit0)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSraHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - - - M2: Read data from memory into internal latch 
  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;

  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform SRa and write back into memory 
  if (ctx->mCycle == M4)
  {
    u8 data = ctx->latchedVal8;
    u8 bit0 = BIT(data, 0);
    u8 bit7 = BIT(data, 7);

    u8 result = (data >> 1) | bit7;

    busWrite(addr, result);

    // - - - fLAGS: Z BASED ON result, N = 0, H = 0, C = OLD BIT 0 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit0)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSwapReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val = cpuGetReg8(instr->reg1);
  
    u8 result = ((val & 0x0F) << 4) | ((val & 0xF0) >> 4);
    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z based on result, N/H/C always 0 
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSwapHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;

  // - - - M3: read
  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform swap and write back to memory 
  if (ctx->mCycle == M4)
  {
    u8 data   = ctx->latchedVal8;
    u8 result = ((data & 0x0F) << 4) | ((data & 0xF0) >> 4);
    busWrite(addr, result);

    // - - - Flags: Z based on result, N/H/C always 0 
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSrlReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val  = cpuGetReg8(instr->reg1);
    u8 bit0 = BIT(val, 0);

    // - - - Logical shift right: C naturally fills bit 7 with 0 for u8 
    u8 result = val >> 1;
    cpuSetReg8(instr->reg1, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C = old bit 0 
    ctx->registers.f = 0;
    if (result == 0) ctx->registers.f |= FLAG_Z;
    if (bit0) ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSrlHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  // - - - M2: Read data from memory into internal latch 
  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;

  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Perform SRL logic and write result back to memory 
  if (ctx->mCycle == M4)
  {
    u8 data   = ctx->latchedVal8;
    u8 bit0   = BIT(data, 0);
    u8 result = data >> 1;

    busWrite(addr, result);

    // - - - Flags: Z based on result, N = 0, H = 0, C = old bit bit0 
    ctx->registers.f = 0;
    if (result == 0)  ctx->registers.f |= FLAG_Z;
    if (bit0)         ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
