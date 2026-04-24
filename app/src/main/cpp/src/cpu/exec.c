#include "cpu/instruction.h"
#include "utils/asserts.h"
#include <cpu/cpu.h>
#include <cpu/ops.h>

/**
 * @file exec.c
 * @brief Executes the currently decoded instruction (M-cycle stepping).
 *
 * Responsibilities:
 * - During CPU_STATE_EXECUTE, route execution to the correct instruction-family stepper.
 * - Provide common helpers for finishing an instruction cleanly.
 *
 * Notes:
 * - Actual instruction logic lives in ops (one file per family).
*/


void cpuFinishInstruction(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - Instruction boundary: apply EI-delay if pending. (EI sets imePending; IME becomes active after the *following* instruction completes.)
  if (ctx->imePending)
  {
    ctx->ime        = true;
    ctx->imePending = false;
  }

  // - - -  Clear per-instruction execution bookkeeping. Keep registers + architectural latches.
  ctx->pcAtFetch = 0;
  ctx->opcode    = 0;
  ctx->isCB      = false;
  ctx->cbOpcode  = 0;
  ctx->instr     = NULL;

  ctx->imm8  = 0;
  ctx->imm16 = 0;

  ctx->mCycleInInstr = 0;
  ctx->microState    = 0;

  ctx->addr         = 0;
  ctx->readData     = 0;
  ctx->hasAddr      = false;
  ctx->hasReadData  = false;

  ctx->conditionPassed = false;

  ctx->state = CPU_STATE_FETCH;
}


// - - - Dispatch - - - 

static inline bool isControlType(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_NOP :
    case IN_STOP:
    case IN_HALT:
    case IN_DI  :
    case IN_EI  :
    case IN_DAA :
    case IN_CPL :
    case IN_SCF :
    case IN_CCF :
      return true;

    default:
      return false;
  }
}

static inline bool isJumpType(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_JP   :
    case IN_JPHL :
    case IN_JR   :
    case IN_CALL :
    case IN_RET  :
    case IN_RETI :
    case IN_RST  :
      return true;

    default:
      return false;
  }
}

static inline bool isLoadType(InstructionType TYPE)
{
  return (TYPE == IN_LD)    || 
         (TYPE == IN_LDH)   || 
         (TYPE == IN_PUSH)  || 
         (TYPE == IN_POP);
}

static inline bool isIncDecType(InstructionType TYPE)
{
  return (TYPE == IN_INC) || 
         (TYPE == IN_DEC);
}

static inline bool isAlu8Type(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_ADD:
    case IN_ADC:
    case IN_SUB:
    case IN_SBC:
    case IN_AND:
    case IN_XOR:
    case IN_OR :
    case IN_CP :
      return true;

    default:
      return false;
  }
}

static inline bool isCbRotateShiftType(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_RLC :
    case IN_RRC :
    case IN_RL  :
    case IN_RR  :
    case IN_SLA :
    case IN_SRA :
    case IN_SWAP:
    case IN_SRL :
      return true;

    default:
      return false;
  }
}

static inline bool isCbBitType(InstructionType TYPE)
{
  return  (TYPE == IN_BIT) || 
          (TYPE == IN_RES) || 
          (TYPE == IN_SET);
}

static inline bool isAlu16Type(const Instruction* INSTR)
{
  // - - - Check if reg1 is a 16-bit register pair
  return (INSTR->reg1 == RT_BC || 
          INSTR->reg1 == RT_DE || 
          INSTR->reg1 == RT_HL || 
          INSTR->reg1 == RT_SP);
}

static inline bool isRotateType(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_RLCA :
    case IN_RRCA :
    case IN_RLA  :
    case IN_RRA  :
      return true;

    default:
      return false;
  }
}

void cpuExecuteStep(void)
{
  CpuContext*     ctx  = cpuGetContext();
  InstructionType type = ctx->instr->type;

  // - - - 1. CB Prefix Family
  if (ctx->isCB)
  {
    if (isCbBitType(type)) 
    { 
      opsCbBitStep(); 
      return; 
    }
    if (isCbRotateShiftType(type))   
    { 
      opsCbRotateShiftStep(); 
      return; 
    }
    return;
  }

  // - - - 2. Control Family (NOP, DI, EI, HALT, STOP, DAA, etc.)
  if (isControlType(type))           
  { 
    opsControlStep(); 
    return; 
  }

  // - - - 3. Jump Family (JP, JR, CALL, RET, RST)
  if (isJumpType(type))
  { 
    opsJumpStep(); 
    return; 
  }

  // - - - 4. Load Family (LD, LDH, PUSH, POP)
  if (isLoadType(type)) 
  { 
    opsLoadStep(); 
    return; 
  }

  // - - - 5. Inc/Dec Family (Both 8 and 16 bit)
  if (isIncDecType(type))   
  { 
    opsIncDecStep(); 
    return; 
  }

  // - - - 6. ALU Family (Logic and Math)
  if (isAlu8Type(type)) 
  {
    // - - - Route 16-bit specials (0xE8, 0xF8) or standard 16-bit ADD (0x09, etc)
    if (isAlu16Type(ctx->instr))
    {
      if (ctx->instr->mode == AM_R_D8)  opsAlu16SpecialStep();
      else                              opsAlu16Step();
    }
    else 
    {
      opsAlu8Step();
    }
    return;
  }

  // - - - 7. Rotate Family
  if (isRotateType(type)) 
  { 
    opsRotateStep(); 
    return; 
  }

  FORGE_LOG_ERROR("Unmapped Instruction Family for Opcode: 0x%02X", ctx->opcode);
  ctx->halted = true;
}
