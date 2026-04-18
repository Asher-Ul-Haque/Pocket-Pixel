#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <bus.h>
#include <utils/bitwise.h>

/**
 * @file cb_prefix.c
 * @brief CB-prefixed instruction family (Rotates, Shifts, BIT, RES, SET).
*/


// - - - Helper: Get/Set CB Target - - -

static u8 getCbVal(RegisterFile* REGS, const Instruction* INSTR) 
{
  if (INSTR->mode == AM_MR_R) return busRead(cpuGetHL(REGS));
  switch (INSTR->reg1) 
  {
    case RT_A: return REGS->a; 
    case RT_B: return REGS->b;
    case RT_C: return REGS->c; 
    case RT_D: return REGS->d;
    case RT_E: return REGS->e; 
    case RT_H: return REGS->h;
    case RT_L: return REGS->l;
    default: 
      return 0;
  }
}

static void setCbVal(RegisterFile* REGS, const Instruction* INSTR, u8 OUT) 
{
  if (INSTR->mode == AM_MR_R) 
  {
    busWrite(cpuGetHL(REGS), OUT);
  } 
  else 
  {
    switch (INSTR->reg1) 
    {
      case RT_A: REGS->a = OUT; break; 
      case RT_B: REGS->b = OUT; break;
      case RT_C: REGS->c = OUT; break; 
      case RT_D: REGS->d = OUT; break;
      case RT_E: REGS->e = OUT; break; 
      case RT_H: REGS->h = OUT; break;
      case RT_L: REGS->l = OUT; break;
      default: 
        break;
    }
  }
}


// - - - Dispatchers - - -

void opsCbRotateShiftStep(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  // - - - (HL) requires an extra M-cycle for the WriteBack
  if (ins->mode == AM_MR_R) 
  {
    if (ctx->microState == 0) { ctx->readData = busRead(cpuGetHL(regs)); ctx->microState = 1; return; }
    if (ctx->microState == 1) { ctx->microState = 2; }
    else 
    { 
      busWrite(cpuGetHL(regs), (u8)ctx->readData); 
      cpuFinishInstruction(); 
      return; 
    }
  }

  u8   val = (ins->mode == AM_MR_R) ? (u8)ctx->readData : getCbVal(regs, ins);
  u8   out = val;
  bool c   = cpuFlagC(regs);

  switch (ins->type) 
  {
    case IN_RLC : cpuSetC(regs, BIT(val, 7)); out = (val << 1) | BIT(val, 7);           break;
    case IN_RRC : cpuSetC(regs, BIT(val, 0)); out = (val >> 1) | (BIT(val, 0) << 7);    break;
    case IN_RL  : cpuSetC(regs, BIT(val, 7)); out = (val << 1) | (c ? 1 : 0);           break;
    case IN_RR  : cpuSetC(regs, BIT(val, 0)); out = (val >> 1) | (c ? 0x80 : 0);        break;
    case IN_SLA : cpuSetC(regs, BIT(val, 7)); out = (val << 1);                         break;
    case IN_SRA : cpuSetC(regs, BIT(val, 7)); out = (val >> 1) | (val & 0x80);          break;
    case IN_SRL : cpuSetC(regs, BIT(val, 0)); out = (val >> 1);                         break;
    case IN_SWAP: 
      out = ((val & 0xF) << 4) | ((val & 0xF0) >> 4); 
      cpuSetC(regs, false); 
      break;
    default: 
      break;
  }

  cpuSetZ(regs, out == 0);
  cpuSetN(regs, false);
  cpuSetH(regs, false);

  if (ins->mode == AM_MR_R) ctx->readData = out;
  else 
  { 
    setCbVal(regs, ins, out); 
    cpuFinishInstruction(); 
  }
}

void opsCbBitStep(void) 
{
  CpuContext*           ctx  = cpuGetContext();
  RegisterFile*         regs = &ctx->registers;
  const Instruction*    ins  = ctx->instr;
  u8                    bit  = ins->param;

  if (ins->mode == AM_MR_R) 
  {
    if (ctx->microState == 0) 
    { 
      ctx->readData = busRead(cpuGetHL(regs)); 
      ctx->microState = 1; 
      return; 
    }
    if (ins->type != IN_BIT && ctx->microState == 1) 
    { 
      // - - - Write blank cycle 
      ctx->microState = 2; 
    }
    else if (ins->type != IN_BIT) 
    { 
      busWrite(cpuGetHL(regs), (u8)ctx->readData); 
      cpuFinishInstruction(); 
      return; 
    }
  }

  u8 val = (ins->mode == AM_MR_R) ? (u8)ctx->readData : getCbVal(regs, ins);

  if (ins->type == IN_BIT) 
  {
    cpuSetZ(regs, !BIT(val, bit));
    cpuSetN(regs, false);
    cpuSetH(regs, true); // - - - BIT always sets H
    cpuFinishInstruction();
  } 
  else 
  {
    BIT_SET(val, bit, (ins->type == IN_SET));
    if (ins->mode == AM_MR_R) ctx->readData = val;
    else 
    { 
      setCbVal(regs, ins, val); 
      cpuFinishInstruction(); 
    }
  }
}
