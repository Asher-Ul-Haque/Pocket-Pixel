#include <utils/bitwise.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>
#include <cpu/ops.h>
#include <cpu/cpu.h>
#include <bus.h>

ExecStatus instrCbBitReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 val      = cpuGetReg8(instr->reg1);
  u8 testBit  = instr->param;

  bool isSet = BIT(val, testBit);

  ctx->registers.f &= ~(FLAG_Z | FLAG_N);
  ctx->registers.f |= FLAG_H;

  if (!isSet) ctx->registers.f |= FLAG_Z;

  return EXEC_STATUS_DONE;
}

ExecStatus instrCbBitHL(void)
{
  CpuContext*        ctx    = cpuGetContext();
  const Instruction* instr  = ctx->currentInstruction;

  // - - - M2: Read and immediately test 
  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;

  if (ctx->mCycle == M3)
  {
    u16 addr = cpuGetReg16(RT_HL);
    u8  val  = busRead(addr);

    ctx->registers.f &= FLAG_C;
    ctx->registers.f |= FLAG_H;

    u8    testBit = instr->param;
    bool  isSet   = BIT(val, testBit);

    if (isSet) ctx->registers.f |= FLAG_Z;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbResReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 val        = cpuGetReg8(instr->reg1);
  u8 bitToReset = instr->param;
  BIT_RESET(val, bitToReset);

  cpuSetReg8(instr->reg1, val);
  return EXEC_STATUS_DONE;
}

ExecStatus instrCbResHL(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;
  u16                 addr  = cpuGetReg16(RT_HL);

  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;
  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Modify and write 
  if (ctx->mCycle == M4)
  {
    u8 val        = ctx->latchedVal8;
    u8 bitToReset = instr->param;
    BIT_RESET(val, bitToReset);
  
    busWrite(addr, val);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCbSetReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 val      = cpuGetReg8(instr->reg1);
  u8 bitToSet = instr->param;

  // - - - set the bit 
  cpuSetReg8(instr->reg1, BIT_SET(val, bitToSet));
  return EXEC_STATUS_DONE;
}

ExecStatus instrCbSetHL(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;
  u16                 addr  = cpuGetReg16(RT_HL);

  if (ctx->mCycle == M2) return EXEC_STATUS_CONTINUE;

  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(addr);
    return EXEC_STATUS_CONTINUE;
  }

  if (ctx->mCycle == M4)
  {
    u8 val = ctx->latchedVal8;
    BIT_SET(val, instr->param);
    busWrite(addr, val);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
