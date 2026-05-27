#include <bus.h>
#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>

ExecStatus instrAndReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 valR = cpuGetReg8(instr->reg2);
  ctx->registers.a &= valR;

  // - - - Flags: Z = depends, N=0, H=1, C=0
  ctx->registers.f = FLAG_H;
  if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrAndHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 data = busRead(cpuGetReg16(RT_HL));
    ctx->registers.a &= data;

    ctx->registers.f = FLAG_H;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrAnd8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 imm = busRead(ctx->registers.programCounter++);
    ctx->registers.a &= imm;

    ctx->registers.f = FLAG_H;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrOrReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 valR = cpuGetReg8(instr->reg2);
  ctx->registers.a |= valR;

  // - - - Flags: z depends, others 0
  ctx->registers.f = 0;
  if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrOrHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 data = busRead(cpuGetReg16(RT_HL));
    ctx->registers.a |= data;

    ctx->registers.f = 0;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrOr8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 imm = busRead(ctx->registers.programCounter++);
    ctx->registers.a |= imm;

    ctx->registers.f = 0;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrXorReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 valR = cpuGetReg8(instr->reg2);
  ctx->registers.a ^= valR;

  // - - - Flags :Z = depends, N = 0, H = 0, C = 0
  ctx->registers.f = 0;
  if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrXorHL(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 data = busRead(cpuGetReg16(RT_HL));
    ctx->registers.a ^= data;

    ctx->registers.f = 0;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrXor8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    u8 imm = busRead(ctx->registers.programCounter++);
    ctx->registers.a ^= imm;

    ctx->registers.f = 0;
    if (ctx->registers.a == 0) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
