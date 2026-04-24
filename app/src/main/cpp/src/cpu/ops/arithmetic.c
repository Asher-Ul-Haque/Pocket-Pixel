#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <bus.h>
#include <utils/bitwise.h>
#include <utils/asserts.h>

/**
 * @file arithmetic.c
 * @brief Full Arithmetic & Logical family (8/16-bit).
 * Covers: ADD, ADC, SUB, SBC, AND, XOR, OR, CP, INC, DEC.
*/

// - - - 8-bit Arithmetic Helpers - - -

static u8 aluAdd8(CpuContext* CTX, u8 A, u8 B, bool CARRY_IN) 
{
  u8  c   = CARRY_IN ? 1 : 0;
  i32 res = A + B + c;

  RegisterFile* r = &CTX->registers;

  cpuSetZ(r, (res & 0xFF) == 0);
  cpuSetN(r, false);
  cpuSetH(r, ((A & 0xF) + (B & 0xF) + c) > 0xF);
  cpuSetC(r, res > 0xFF);

  return (u8)res;
}

static u8 aluSub8(CpuContext* CTX, u8 A, u8 B, bool CARRY_IN) 
{
  u8  c     = CARRY_IN ? 1 : 0;
  i32 res   = A - B - c;

  RegisterFile* r = &CTX->registers;

  cpuSetZ(r, (res & 0xFF) == 0);
  cpuSetN(r, true);
  cpuSetH(r, (A & 0xF) < ((B & 0xF) + c));
  cpuSetC(r, res < 0);

  return (u8)res;
}


// - - - 16-bit Arithmetic Helpers - - -

static u16 aluAdd16(CpuContext* CTX, u16 A, u16 B) 
{
  i32           res = A + B;
  RegisterFile* r   = &CTX->registers;

  cpuSetN(r, false);
 
  // - - - 16-bit Half-Carry is from bit 11 to 12
  cpuSetH(r, ((A & 0x0FFF) + (B & 0x0FFF)) > 0x0FFF);
  cpuSetC(r, res > 0xFFFF);

  return (u16)res;
}

/**
 * Special case: ADD SP, e8 (Opcode 0xE8)
 * Calculations for H and C are based on the lower 8 bits.
*/
static u16 aluAddSpE8(CpuContext* CTX, u16 SP, i8 REL) 
{
  RegisterFile* r   = &CTX->registers;
  u32           res = SP + REL;

  cpuSetZ(r, false);
  cpuSetN(r, false);
  cpuSetH(r, ((SP & 0xF) + (REL & 0xF)) > 0xF);
  cpuSetC(r, ((SP & 0xFF) + (REL & 0xFF)) > 0xFF);
  return (u16)res;
}


// - - - Operand Fetch Helper - - -

static u16 readOperand(CpuContext* CTX, RegisterFile* REGS, const Instruction* INSTRUCTION) 
{
  if (INSTRUCTION->mode == AM_R_MR || INSTRUCTION->mode == AM_MR_R) 
  {
    return busRead(cpuGetHL(REGS));
  } 
  else if (INSTRUCTION->mode == AM_R_D8 || INSTRUCTION->mode == AM_D8) 
  {
    return CTX->imm8;
  } 
  else if (INSTRUCTION->reg2 != RT_NONE) 
  {
    // - - - Generic 8-bit reg fetch
    switch(INSTRUCTION->reg2) 
    {
      case RT_A: return REGS->a; 
      case RT_B: return REGS->b;
      case RT_C: return REGS->c;
      case RT_D: return REGS->d;
      case RT_E: return REGS->e; 
      case RT_H: return REGS->h;
      case RT_L: return REGS->l;
      default: return 0;
    }
  }
  return 0;
}


// - - - Entry Points - - -

void opsAlu8Step(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  // - - - Handle M-cycle for (HL) or immediate
  if (ctx->microState == 0 && (ins->mode == AM_R_MR || ins->mode == AM_R_D8)) 
  {
    ctx->readData   = readOperand(ctx, regs, ins);
    ctx->microState = 1;
    return;
  }

  u8 val = (u8)((ins->mode == AM_R_MR || ins->mode == AM_R_D8) ? ctx->readData : readOperand(ctx, regs, ins));

  switch (ins->type) 
  {
    case IN_ADD: regs->a = aluAdd8(ctx, regs->a, val, false);           break;
    case IN_ADC: regs->a = aluAdd8(ctx, regs->a, val, cpuFlagC(regs));  break;
    case IN_SUB: regs->a = aluSub8(ctx, regs->a, val, false);           break;
    case IN_SBC: regs->a = aluSub8(ctx, regs->a, val, cpuFlagC(regs));  break;
    case IN_AND: 
        regs->a &= val; 
        cpuSetZ(regs, regs->a == 0); cpuSetN(regs, false); cpuSetH(regs, true); cpuSetC(regs, false); 
        break;
    case IN_XOR: 
        regs->a ^= val; 
        cpuSetZ(regs, regs->a == 0); cpuSetN(regs, false); cpuSetH(regs, false); cpuSetC(regs, false); 
        break;
    case IN_OR:  
        regs->a |= val; 
        cpuSetZ(regs, regs->a == 0); cpuSetN(regs, false); cpuSetH(regs, false); cpuSetC(regs, false); 
        break;
    case IN_CP:  
      aluSub8(ctx, regs->a, val, false); 
      break;
    default: break;
  }
  cpuFinishInstruction();
}

void opsRotateStep(void)
{
  CpuContext* ctx = cpuGetContext();
  RegisterFile* regs = &ctx->registers;
  u8 a = regs->a;
  u8 carry = cpuFlagC(regs) ? 1 : 0;

  switch (ctx->instr->type)
  {
    case IN_RLCA: // ROTATE A left; Bit 7 to carry, and bit 0
      cpuSetC(regs, (a & 0x80) != 0);
      regs->a = (a << 1) | (a >> 7);
      break;

    case IN_RRCA: // Rotate A right; Bit 0 to carry and Bit 7
      cpuSetC(regs, (a & 0x01) != 0);
      regs->a = (a >> 1) | (a << 7);
      break;

    case IN_RLA: // Rotate A Left through carry
      {
        bool newCarry = (a & 0x80) != 0;
        regs->a = (a << 1) | carry;
        cpuSetC(regs, newCarry);
      }
      break;

    case IN_RRA: // Rotate A Right through carry
      {
        bool newCarry = (a & 0x01) != 0;
        regs->a = (a >> 1) | (carry << 7);
        cpuSetC(regs, newCarry);
      }
      break;

    default: break;
  }
  cpuSetZ(regs, false);
  cpuSetN(regs, false);
  cpuSetH(regs, false);
  cpuFinishInstruction();
}

void opsIncDecStep(void) 
{
  CpuContext*           ctx     = cpuGetContext();
  RegisterFile*         regs    = &ctx->registers;
  const Instruction*    ins     = ctx->instr;

  bool is16 = (ins->reg1 == RT_BC || ins->reg1 == RT_DE || ins->reg1 == RT_HL || ins->reg1 == RT_SP);

  // - - - 16-bit INC/DEC (No flags affected)
  if (is16) 
  {
    if (ctx->microState == 0) { ctx->microState = 1; return; } // - - - Internal cycle
    u16 val;
    switch(ins->reg1) 
    {
      case RT_BC: val = cpuGetBC(regs); (ins->type == IN_INC) ? cpuSetBC(regs, ++val) : cpuSetBC(regs, --val); break;
      case RT_DE: val = cpuGetDE(regs); (ins->type == IN_INC) ? cpuSetDE(regs, ++val) : cpuSetDE(regs, --val); break;
      case RT_HL: val = cpuGetHL(regs); (ins->type == IN_INC) ? cpuSetHL(regs, ++val) : cpuSetHL(regs, --val); break;
      case RT_SP: (ins->type == IN_INC) ? regs->stackPointer++ : regs->stackPointer--; break;
      default: 
        break;
    }
    cpuFinishInstruction();
    return;
  }

  // - - - 8-bit INC/DEC (HL) - 3 M-cycles (Read-Modify-Write)
  if (ins->reg1 == RT_HL) 
  {
    u16 addr = cpuGetHL(regs);
    if (ctx->microState == 0) { ctx->readData = busRead(addr); ctx->microState = 1; return; }
    if (ctx->microState == 1) 
    {
      u8 val = (u8)ctx->readData;
      if (ins->type == IN_INC) 
      {
        cpuSetH(regs, (val & 0xF) == 0xF);
        val++; cpuSetN(regs, false);
      } 
      else 
      {
        cpuSetH(regs, (val & 0xF) == 0);
        val--; 
        cpuSetN(regs, true);
      }
      cpuSetZ(regs, val == 0);
      ctx->readData   = val;
      ctx->microState = 2;
      return;
    }
    busWrite(addr, (u8)ctx->readData);
    cpuFinishInstruction();
    return;
  }

  // - - - 8-bit Register INC/DEC
  u8* r   = (ins->reg1 == RT_A) ? &regs->a : (ins->reg1 == RT_B) ? &regs->b : (ins->reg1 == RT_C) ? &regs->c : 
            (ins->reg1 == RT_D) ? &regs->d : (ins->reg1 == RT_E) ? &regs->e : (ins->reg1 == RT_H) ? &regs->h : &regs->l;
  u8  val = *r;
  if (ins->type == IN_INC) 
  {
    cpuSetH(regs, (val & 0xF) == 0xF);
    val++;  
    cpuSetN(regs, false);
  } 
  else 
  {
    cpuSetH(regs, (val & 0xF) == 0);
    val--; 
    cpuSetN(regs, true);
  }
  cpuSetZ(regs, val == 0);
  *r = val;
  cpuFinishInstruction();
}

/// @brief Handles 16-bit ADD (HL, rr) and the special ADD SP, e8.
void opsAlu16Step(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  if (ctx->microState == 0) { ctx->microState = 1; return; } // - - - Internal cycle

  if (ins->type == IN_ADD) 
  {
    if (ins->reg1 == RT_SP) 
    { 
      // - - - ADD SP, e8
      regs->stackPointer = aluAddSpE8(ctx, regs->stackPointer, (i8)ctx->imm8);
    } 
    else 
    { 
      // - - - ADD HL, rr
      u16 val = (ins->reg2 == RT_BC) ? cpuGetBC(regs) : (ins->reg2 == RT_DE) ? cpuGetDE(regs) : 
                (ins->reg2 == RT_HL) ? cpuGetHL(regs) : regs->stackPointer;
      cpuSetHL(regs, aluAdd16(ctx, cpuGetHL(regs), val));
    }
  }
  cpuFinishInstruction();
}

void opsAlu16SpecialStep(void) 
{
  CpuContext*        ctx  = cpuGetContext();
  RegisterFile*      regs = &ctx->registers;
  const Instruction* ins  = ctx->instr;

  // - - - Both E8 (ADD SP, e8) and F8 (LD HL, SP + e8) take 3 M-cycles
  if (ctx->microState == 0) { ctx->microState = 1; return; } // - - - Internal cycle
  if (ctx->microState == 1) { ctx->microState = 2; return; } // - - - Internal cycle
    
  // - - - Logic for ADD SP, e8 and LD HL, SP+e8
  i8 offset = (i8)ctx->imm8;
  u16 sp = regs->stackPointer;
    
  // - - - Flags are always calculated based on the lower 8-bit addition
  cpuSetZ(regs, false);
  cpuSetN(regs, false);
  cpuSetH(regs, ((sp & 0xF) + (offset & 0xF)) > 0xF);
  cpuSetC(regs, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);

  if (ins->type == IN_ADD) 
  {
    regs->stackPointer = sp + offset;
  } 
  else 
  { 
    // - - - IN_LD (LD HL, SP+e8)
    cpuSetHL(regs, sp + offset);
  }
    
  cpuFinishInstruction();
}


/**
 * @brief Opcode 0xE8: ADD SP, e8
 * Adds a signed 8-bit immediate to the Stack Pointer.
 * Flags: Z=0, N=0, H/C from lower 8-bit sum.
 * Takes 4 M-cycles.
*/
void opsAddSpE8Step(void) 
{
  CpuContext* ctx = cpuGetContext();
  RegisterFile* regs = &ctx->registers;

  // - - - M2-M3: Internal cycles for timing/ALU
  if (ctx->microState < 2) 
  {
    ctx->microState++;
    return;
  }

  // - - - M4: Execution and Flag calculation
  i8  rel = (i8)ctx->imm8;
  u16 sp  = regs->stackPointer;

  cpuSetZ(regs, false);
  cpuSetN(regs, false);

  // - - - H/C bits are determined by the 8-bit addition (bit 3 and bit 7)
  cpuSetH(regs, ((sp & 0xF)  + (rel & 0xF))  > 0xF);
  cpuSetC(regs, ((sp & 0xFF) + (rel & 0xFF)) > 0xFF);

  regs->stackPointer = sp + rel;
  cpuFinishInstruction();
}

/**
 * @brief Opcode 0xF8: LD HL, SP + e8
 * Loads HL with the result of SP + signed 8-bit immediate.
 * Flags: Z=0, N=0, H/C from lower 8-bit sum.
 * Takes 3 M-cycles.
*/
void opsLdHlSpE8Step(void) 
{
  CpuContext*   ctx  = cpuGetContext();
  RegisterFile* regs = &ctx->registers;

  // - - - M2: Internal cycle
  if (ctx->microState == 0) 
  {
    ctx->microState = 1;
    return;
  }

  // - - - M3: Execution
  i8  rel = (i8)ctx->imm8;
  u16 sp  = regs->stackPointer;

  cpuSetZ(regs, false);
  cpuSetN(regs, false);
  cpuSetH(regs, ((sp & 0xF)  + (rel & 0xF))  > 0xF);
  cpuSetC(regs, ((sp & 0xFF) + (rel & 0xFF)) > 0xFF);

  cpuSetHL(regs, sp + rel);
  cpuFinishInstruction();
}


