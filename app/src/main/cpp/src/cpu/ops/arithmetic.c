#include <bus.h>
#include <cpu/alu.h>
#include <cpu/instruction.h>
#include <cpu/registers.h>
#include <cpu/cpu.h>
#include <cpu/ops.h>

ExecStatus instrAddAReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - 1. Get operands 
  u8 valA = ctx->registers.a;
  u8 valR = cpuGetReg8(instr->reg2);

  // - - - 2. Perform stateless ALU operation
  AluResult8 res = aluAdd8(valA, valR, false);

  // - - - 3. Commit result to accumulator
  ctx->registers.a = res.result;

  // - - - 4. Update flag 
  ctx->registers.f = 0;
  if (res.zero)       ctx->registers.f |= FLAG_Z;
  if (res.halfCarry)  ctx->registers.f |= FLAG_H;
  if (res.carry)      ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrAddAHL(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Bus access and ALU operation 
  if (ctx->mCycle == M2)
  {
    u16         addr  = cpuGetReg16(RT_HL);
    u8          data  = busRead(addr);
    AluResult8  res   = aluAdd8(ctx->registers.a, data, false);

    // - - - commit and update flags 
    ctx->registers.a = res.result;
    ctx->registers.f = 0;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAddA8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Fetch operand from PC, cacl, commit and update 
  if (ctx->mCycle == M2)
  {
    u8          imm = busRead(ctx->registers.programCounter++);
    AluResult8  res = aluAdd8(ctx->registers.a, imm, false);

    ctx->registers.a = res.result;
    ctx->registers.f = 0;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAdcReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  bool        cIn   = (ctx->registers.f & FLAG_C) != 0;
  u8          valR  = cpuGetReg8(instr->reg2);
  AluResult8  res   = aluAdd8(ctx->registers.a, valR, cIn);

  ctx->registers.a = res.result;
  ctx->registers.f = 0;
  if (res.zero)       ctx->registers.f |= FLAG_Z;
  if (res.halfCarry)  ctx->registers.f |= FLAG_H;
  if (res.carry)      ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrAdc8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8          imm = busRead(ctx->registers.programCounter++);
    bool        cIn = (ctx->registers.f & FLAG_C) != 0;
    AluResult8  res = aluAdd8(ctx->registers.a, imm, cIn);

    ctx->registers.a = res.result;
    ctx->registers.f = 0;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAdcHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8          data  = busRead(cpuGetReg16(RT_HL));
    bool        cIn   = (ctx->registers.f & FLAG_C) != 0;
    AluResult8  res   = aluAdd8(ctx->registers.a, data, cIn);

    ctx->registers.a = res.result;
    ctx->registers.f = 0;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }
  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrSubReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8          valR  = cpuGetReg8(instr->reg2);
  AluResult8  res   = aluSub8(ctx->registers.a, valR, false);
  
  ctx->registers.a = res.result;
  ctx->registers.f = FLAG_N;
  if (res.zero)       ctx->registers.f |= FLAG_Z;
  if (res.halfCarry)  ctx->registers.f |= FLAG_H;
  if (res.carry)      ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrSubHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8          data  = busRead(cpuGetReg16(RT_HL));
    AluResult8  res   = aluSub8(ctx->registers.a, data, false);

    ctx->registers.a = res.result;
    ctx->registers.f = FLAG_N;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }
  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrSub8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8          imm = busRead(ctx->registers.programCounter++);
    AluResult8  res = aluSub8(ctx->registers.a, imm, false);

    ctx->registers.a = res.result;
    ctx->registers.f = FLAG_N;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrSbcReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8    valR  = cpuGetReg8(instr->reg2);
  bool  bIn   = (ctx->registers.f & FLAG_C) != 0;

  AluResult8 res    = aluSub8(ctx->registers.a, valR, bIn);
  ctx->registers.a  = res.result;

  // - - - Flags: rebuild from scratch for SBC (N=1)
  ctx->registers.f = FLAG_N;
  if (res.zero)       ctx->registers.f |= FLAG_Z;
  if (res.halfCarry)  ctx->registers.f |= FLAG_H;
  if (res.carry)      ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrSbcHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8    data  = busRead(cpuGetReg16(RT_HL));
    bool  bIn   = (ctx->registers.f & FLAG_C) != 0;

    AluResult8 res = aluSub8(ctx->registers.a, data, bIn);

    ctx->registers.a = res.result;
    ctx->registers.f = FLAG_N;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }
  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrSbc8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8    imm = busRead(ctx->registers.programCounter++);
    bool  bIn = (ctx->registers.f & FLAG_C) != 0;

    AluResult8 res = aluSub8(ctx->registers.a, imm, bIn);

    ctx->registers.a = res.result;
    ctx->registers.f = FLAG_N;

    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCompareReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8          valR  = cpuGetReg8(instr->reg2);
  AluResult8  res   = aluSub8(ctx->registers.a, valR, false);

  // - - - Update flags only 
  ctx->registers.f = FLAG_N;
  if (res.zero)       ctx->registers.f |= FLAG_Z;
  if (res.halfCarry)  ctx->registers.f |= FLAG_H;
  if (res.carry)      ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrCompareHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 data = busRead(cpuGetReg16(RT_HL));
    AluResult8 res = aluSub8(ctx->registers.a, data, false);

    ctx->registers.f = FLAG_N;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCompare8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8          imm = busRead(ctx->registers.programCounter++);
    AluResult8  res = aluSub8(ctx->registers.a, imm, false);

    ctx->registers.f = FLAG_N;
    if (res.zero)       ctx->registers.f |= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrIncrementReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8          val = cpuGetReg8(instr->reg1);
  AluResult8  res = aluAdd8(val, 1, false);

  cpuSetReg8(instr->reg1, res.result);

  // - - - Flags: Z, depend on result; N is reset; C is preserved 
  u8 flag = ctx->registers.f & FLAG_C;
  if (res.zero)       flag |= FLAG_Z;
  if (res.halfCarry)  flag |= FLAG_H;
  ctx->registers.f = flag;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrDecrementReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8          val = cpuGetReg8(instr->reg1);
  AluResult8  res = aluSub8(val, 1, false);
  cpuSetReg8(instr->reg1, res.result);

  // - - - Flags: Z, H depen on result; N is set; C is preserved 
  u8 flag = (ctx->registers.f & FLAG_C) | FLAG_N;
  if (res.zero)       flag |= FLAG_Z;
  if (res.halfCarry)  flag |= FLAG_H;
  ctx->registers.f          = flag;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrIncrementHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: ALU and Write
  if (ctx->mCycle == M3)
  {
    AluResult8 res = aluAdd8(ctx->latchedVal8, 1, false);
    busWrite(addr, res.result);

    // - - - Flags: Preserver C 
    u8 flag = ctx->registers.f & FLAG_C;
    if (res.zero)       flag |= FLAG_Z;
    if (res.halfCarry)  flag |= FLAG_H;
    ctx->registers.f          = flag;

    return EXEC_STATUS_DONE;
  }
  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrDecrementHL(void)
{
  CpuContext* ctx   = cpuGetContext();
  u16         addr  = cpuGetReg16(RT_HL);

  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: ALU and Write 
  if (ctx->mCycle == M3)
  {
    AluResult8 res = aluSub8(ctx->latchedVal8, 1, false);
    busWrite(addr, res.result);

    // - - - Flags: Preserve C, Set N 
    u8 flag = (ctx->registers.f & FLAG_C) | FLAG_N;
    if (res.zero)       flag |= FLAG_Z;
    if (res.halfCarry)  flag |= FLAG_H;
    ctx->registers.f = flag;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCcf(void) 
{
  CpuContext* ctx = cpuGetContext();

  u8 zFlag = ctx->registers.f & FLAG_Z;
  u8 cFlag = ctx->registers.f & FLAG_C;

  // - - - Update F: Z stays, N and H become 0, C is inverted
  ctx->registers.f = zFlag; 
  if (!cFlag) ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrScf(void)
{
  CpuContext* ctx   = cpuGetContext();

  // - - - Update F: Z stays, N and H become 0, C is set
  ctx->registers.f &= FLAG_Z;
  ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrDaa(void)
{
  CpuContext* ctx      = cpuGetContext();
  u8          a        = ctx->registers.a;
  u8          adjust   = 0;
  bool        isSub    = (ctx->registers.f & FLAG_N) != 0;
  bool        halfIn   = (ctx->registers.f & FLAG_H) != 0;
  bool        carryOut = (ctx->registers.f & FLAG_C) != 0;

  if (!isSub)
  {
    if (halfIn || (a & 0x0F) > 0x09) adjust |= 0x06;
    if (carryOut || a > 0x99)
    {
      adjust   |= 0x60;
      carryOut = true;
    }
    a += adjust;
  }
  else
  {
    if (halfIn)   adjust |= 0x06;
    if (carryOut) adjust |= 0x60;
    a -= adjust;
  }

  ctx->registers.a = a;

  // - - - DAA: N preserved, H cleared, Z from final A, C explicit.
  ctx->registers.f &= FLAG_N;
  if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;
  if (carryOut)              ctx->registers.f |= FLAG_C;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrCpl(void)
{
  CpuContext* ctx = cpuGetContext();

  ctx->registers.a  = ~ctx->registers.a;
  ctx->registers.f |= (FLAG_N | FLAG_H);

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrIncrementRegReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: rr <- rr + 1 
  if (ctx->mCycle == M2) 
  {
    u16 val = cpuGetReg16(instr->reg1);
    cpuSetReg16(instr->reg1, val + 1);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrDecrementRegReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: rr <- rr - 1 
  if (ctx->mCycle == M2) 
  {
    u16 val = cpuGetReg16(instr->reg1);
    cpuSetReg16(instr->reg1, val - 1);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAddHlRegReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u16 hl = cpuGetReg16(RT_HL);
    u16 rr = cpuGetReg16(instr->reg2);

    // - - - Use the 16-bit ALU helper for HL addition 
    AluResult16 res = aluAdd16(hl, rr);
    cpuSetReg16(RT_HL, res.result);

    // - - - Flags: Z is preserved, N is reset, H and C depend on bit 11 and 15 
    ctx->registers.f &= FLAG_Z;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAddSpE8(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Fetch signed immediate e 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Internal ALU wait 
  if (ctx->mCycle == M3) return EXEC_STATUS_CONTINUE;

  // - - - M4: Finalize the result and flags 
  if (ctx->mCycle == M4)
  {
    i8 offset = (i8) ctx->latchedVal8;
    u16 sp = ctx->registers.stackPointer;

    // - - - Use the specialized SP ALU helper we made for LD HL, Sp+e8 
    AluResult16 res = aluAdd16Sp(sp, offset);

    // - - - Flags: Z = 0, N = 0, H, and C, from the 8 bit carry logic 
    ctx->registers.f = 0;
    if (res.halfCarry)  ctx->registers.f |= FLAG_H;
    if (res.carry)      ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
