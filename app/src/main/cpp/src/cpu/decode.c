/**
 * @file cpu_decode.c
 * @brief M-cycle fetch/decode state machine (SM83).
 *
 * This file advances the CPU through:
 *   CPU_STATE_FETCH -> (CPU_STATE_FETCH_CB)? -> CPU_STATE_DECODE -> CPU_STATE_EXECUTE
 *
 * Design contract:
 * - Decode is responsible for fetching opcode byte(s) and any immediate operand bytes
 *   (imm8/imm16) and advancing PC accordingly.
 * - When decode transitions to CPU_STATE_EXECUTE, PC points at the next sequential
 *   instruction (i.e., after immediate bytes).
 * - Execution uses ctx->info + ctx->imm8/ctx->imm16 and may modify PC for branches/jumps.
 *
 * NOTE: This is an M-cycle stepper. It performs at most one bus read per call.
*/

#include <cpu/cpu.h>
#include <bus.h>


// - - - Forward declaration of internal helpers 

static void clearInstructionTransientForDecode(CpuContext* CTX);
static bool instructionNeedsImm8(AddressMode MODE);
static bool instructionNeedsImm16(AddressMode MODE);

static void clearInstructionTransientForDecode(CpuContext* CTX)
{
  CTX->pcAtFetch = 0;

  CTX->opcode   = 0;
  CTX->isCB     = false;
  CTX->cbOpcode = 0;

  CTX->instr = NULL;

  CTX->imm8  = 0;
  CTX->imm16 = 0;

  CTX->mCycleInInstr = 0;
  CTX->microState    = 0;

  CTX->addr         = 0;
  CTX->readData     = 0;
  CTX->hasAddr      = false;
  CTX->hasReadData  = false;

  CTX->conditionPassed = false;
}

static bool instructionNeedsImm8(AddressMode MODE)
{
  switch (MODE)
  {
    case AM_D8    :
    case AM_R_D8  :
    case AM_MR_D8 :
    case AM_A8_R  :
    case AM_R_A8  :
      return true;

    default:
      return false;
  }
}

static bool instructionNeedsImm16(AddressMode MODE)
{
  switch (MODE)
  {
    case AM_D16  :
    case AM_R_D16:
    case AM_A16_R:
    case AM_R_A16:
      return true;

    default:
      return false;
  }
}


void cpuDecodeStep(void)
{
  CpuContext* ctx = cpuGetContext();

  switch (ctx->state)
  {
    case CPU_STATE_FETCH:
      clearInstructionTransientForDecode(ctx);
      
      // - - - HALT bug: if latch is set, next fetch repeats last byte (PC not advanced properly)
      if (ctx->haltBug)
      {
        ctx->registers.programCounter--;
        ctx->haltBug = false;
      }

      ctx->pcAtFetch    = ctx->registers.programCounter;
      ctx->opcode       = busRead(ctx->registers.programCounter);
      ctx->registers.programCounter++;

      if (ctx->opcode == 0xCB)
      {
        ctx->isCB  = true;
        ctx->state = CPU_STATE_FETCH_CB;
      }
      else 
      {
        ctx->state = CPU_STATE_DECODE;
      }
      break;


    case CPU_STATE_FETCH_CB:
      ctx->cbOpcode = busRead(ctx->registers.programCounter);
      ctx->state    = CPU_STATE_DECODE;
      ctx->registers.programCounter++;
      break;

    
    case CPU_STATE_DECODE:
      if (ctx->isCB) ctx->instr = instructionGetByCBOpcode(ctx->cbOpcode);
      else           ctx->instr = instructionGetByOpcode(ctx->opcode);
      
      FORGE_ASSERT_DEBUG(ctx->instr, "Instruction must always return a valid pointer");
    
      /* Prefetch immediates.
         We use ctx->microState as a tiny decode-substate:
           0 = none fetched yet
           1 = fetched imm8
           2 = fetched imm16 low byte (stored temporarily in imm16 low)
           3 = fetched imm16 high byte (complete)
         After immediates are done, transition to EXECUTE and reset microState for execution. 
      */ 
      const AddressMode mode = ctx->instr->mode;
      if (instructionNeedsImm8(mode))
      {
        if (ctx->microState == 0)
        {
          ctx->imm8         = busRead(ctx->registers.programCounter);
          ctx->microState   = 1;
          ctx->registers.programCounter++;
          return;
        }
      } 

      else if (instructionNeedsImm16(mode))
      {
        if (ctx->microState == 0)
        {
          ctx->imm16 = busRead(ctx->registers.programCounter);
          ctx->registers.programCounter++;
          ctx->microState = 2;
          return;
        }

        if (ctx->microState == 2)
        {
          u8 hi = busRead(ctx->registers.programCounter);
          ctx->registers.programCounter++;
          ctx->imm16     |= (u16) ((u16)hi << 8);
          ctx->microState = 3;
          return;
        }
      }

      ctx->state         = CPU_STATE_EXECUTE;
      ctx->mCycleInInstr = 0;
      ctx->microState    = 0;
      break;
      

    default:
      FORGE_ASSERT_DEBUG(true, "cpuDecodeStep called in non-decode state");
  }
}
