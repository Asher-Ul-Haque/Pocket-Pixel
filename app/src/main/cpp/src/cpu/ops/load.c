#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <bus.h>
#include <cpu/instruction.h>
#include <utils/asserts.h>

/**
 * @file load.c
 * @brief Complete Load/store instruction family.
*/

static u8* reg8Ptr(RegisterFile* REGISTER, RegType TYPE) 
{
  switch (TYPE) 
  {
    case RT_A: return &REGISTER->a; 
    case RT_B: return &REGISTER->b;
    case RT_C: return &REGISTER->c; 
    case RT_D: return &REGISTER->d;
    case RT_E: return &REGISTER->e; 
    case RT_H: return &REGISTER->h;
    case RT_L: return &REGISTER->l;
    default: 
      return NULL;
  }
}

static void writeReg16(RegisterFile* REGISTER, RegType TYPE, u16 VALUE) 
{
  switch (TYPE) 
  {
    case RT_BC: cpuSetBC(REGISTER, VALUE);        break;
    case RT_DE: cpuSetDE(REGISTER, VALUE);        break;
    case RT_HL: cpuSetHL(REGISTER, VALUE);        break;
    case RT_SP: REGISTER->stackPointer   = VALUE; break;
    case RT_PC: REGISTER->programCounter = VALUE; break;
    default: 
      break;
  }
}


// - - - Specialized Step Functions - - -

/**
 * @brief Opcode 0x08: LD (a16), SP
 * Writes the 16-bit Stack Pointer to an absolute 16-bit address.
 * Takes 5 M-cycles (Fetch + 2 for imm16 + 2 for 16-bit write).
*/
void opsLoadSpToAddrStep(void) 
{
  CpuContext*   ctx  = cpuGetContext();
  RegisterFile* regs = &ctx->registers;

  // - - - M1-M3: Handled by Fetch/Decode (Opcode + Imm16)
  // - - - M4: Write SP Low Byte
  if (ctx->microState == 0) 
  {
    busWrite(ctx->imm16, cpuLo8(regs->stackPointer));
    ctx->microState = 1;
    return;
  }
  
  // - - - M5: Write SP High Byte
  if (ctx->microState == 1) 
  {
    busWrite(ctx->imm16 + 1, cpuHi8(regs->stackPointer));
  }
    
  cpuFinishInstruction();
}

void opsLoadHighStep(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  if (ctx->microState == 0) 
  {
    // - - - 1. Determine address ($FF00 + imeddiate OR $FF00 + C)
    u16 addr = 0xFF00 + ((ins->mode == AM_R_A8 || ins->mode == AM_A8_R) ? ctx->imm8 : regs->c);

    // - - - 2, Direction check based on Addressing Mode

    // - - - Read modes: AM_R_A8 or AM_R_MR_C, Register 1 is the destination
    if (ins->mode == AM_R_A8 || ins->mode == AM_R_MR_C)
    {
      regs->a = busRead(addr);
    }
    
    // - - - Write mode: AM_A8_R or AM_MR_C (Memory is the destination)
    else 
    {
      busWrite(addr, regs->a);
    }

    ctx->microState = 1;
    return;
  }
  cpuFinishInstruction();
}


// - - - Dispatcher - - -

void opsLoadStep(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  ctx->mCycleInInstr++;

  switch (ins->mode) 
  {
    // - - - LD r, r (1 M-cycle)
    case AM_R_R: 
      *reg8Ptr(regs, ins->reg1) = *reg8Ptr(regs, ins->reg2);
      cpuFinishInstruction();
      break;

    // - - - LD, r, n8 (2 M-cycles)
    case AM_R_D8: 
      if (ctx->microState == 0) 
      { 
        ctx->microState = 1; 
        return; 
      }
      *reg8Ptr(regs, ins->reg1) = ctx->imm8;
      cpuFinishInstruction();
      break;

    // - - - LD rr, n16 (3 M-cycles)
    case AM_R_D16: 
      if (ctx->microState == 0) 
      { 
        ctx->microState = 1; 
        return; 
      }
      writeReg16(regs, ins->reg1, ctx->imm16);
      cpuFinishInstruction();
      break;

    // - - - LD (HL), r or  LD (BC), A etc. (2 M-cycles)
    case AM_MR_R:
      if (ctx->microState == 0) 
      { 
        ctx->microState = 1; 
        return; 
      }
      u16 destAddr =  (ins->reg1 == RT_HL) ? cpuGetHL(regs) : 
                      (ins->reg1 == RT_BC) ? cpuGetBC(regs) : cpuGetDE(regs);
      busWrite(destAddr, *reg8Ptr(regs, ins->reg2));

      // - - - Hanlde HL+/HL- (param is 1 or -1)
      if (ins->reg1 == RT_HL && ins->param != 0)
      {
        cpuSetHL(regs, cpuGetHL(regs) + (i8)ins->param);
      }
      cpuFinishInstruction();
      break;

    // - - - LD r, (HL) or LD A, (BC) etc. (2 M-cycles)
    case AM_R_MR: 
      if (ctx->microState == 0) 
      { 
        ctx->microState = 1; 
        return; 
      }
      u16 srcAddr = (ins->reg2 == RT_HL) ? cpuGetHL(regs) : 
                    (ins->reg2 == RT_BC) ? cpuGetBC(regs) : cpuGetDE(regs);
      *reg8Ptr(regs, ins->reg1) = busRead(srcAddr);

      // - - - Handle HL+/HL- (param is 1 or -1)
      if (ins->reg2 == RT_HL && ins->param != 0)
      {
        cpuSetHL(regs, cpuGetHL(regs) + (i8)ins->param);
      }

      cpuFinishInstruction();
      break;

    // - - - LD (a16), A or LD (a16), SP
    case AM_A16_R: 
      if (ins->reg1 == RT_SP) opsLoadSpToAddrStep();
      else 
      {
        if (ctx->microState == 0) 
        { 
          ctx->microState = 1; 
          return; 
        }
        busWrite(ctx->imm16, regs->a);
        cpuFinishInstruction();
      }
      break;

    // - - - LD A, (a16)
    case AM_R_A16: 
      if (ctx->microState == 0) 
      { 
        ctx->microState = 1; 
        return; 
      }
      regs->a = busRead(ctx->imm16);
      cpuFinishInstruction();
      break;

    case AM_A8_R: 
    case AM_R_A8: 
    case AM_MR_C: 
    case AM_R_MR_C:
      opsLoadHighStep();
      break;

    default:
      FORGE_ASSERT_DEBUG(false, "Unsupported addressing mode in opsLoadStep");
      break;
  }
}
