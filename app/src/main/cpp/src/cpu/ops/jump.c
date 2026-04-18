#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <bus.h>

/**
 * @file ops_jump.c
 * @brief Jump/call/return/rst instruction family (M-cycle stepping).
 *
 * Covers:
 * - JP a16 / JP cc,a16 / JP (HL)
 * - JR e8 / JR cc,e8
 * - CALL a16 / CALL cc,a16
 * - RET / RET cc
 * - RETI
 * - RST n
 *
 * Stepping rules:
 * - At most ONE bus operation per M-cycle (stack read/write helpers do exactly one bus op).
 * - Immediates are prefetched during decode into ctx->imm8/imm16.
 *
 * We use ctx->microState for per-instruction microcode.
*/


static void stepJP(CpuContext* CTX)
{
  // - - -  JP a16 / JP cc,a16
  if (CTX->microState == 0)
  {
    CTX->conditionPassed = cpuEvalCond(CTX->instr->cond);
    CTX->microState      = 1;
    return; 
  }

  if (!CTX->conditionPassed)
  {
    cpuFinishInstruction();
    return;
  }

  if (CTX->microState == 1)
  {
    CTX->microState = 2;
    return;
  }

  CTX->registers.programCounter = CTX->imm16;
  cpuFinishInstruction();
}

static void stepJPHL(CpuContext* CTX)
{
  // - - - JP (HL) unconditional
  if (CTX->microState == 0)
  {
    CTX->microState = 1;
    return;
  }

  CTX->registers.programCounter = cpuGetHL(&CTX->registers);
  cpuFinishInstruction();
}

static void stepJR(CpuContext* CTX)
{
  // - - - JR e8 / JR cc,e8; imm8 is signed offset
  if (CTX->microState == 0)
  {
    CTX->conditionPassed = cpuEvalCond(CTX->instr->cond);
    CTX->microState      = 1;
    return;
  }

  if (!CTX->conditionPassed)
  {
    cpuFinishInstruction();
    return;
  }

  if (CTX->microState == 1)
  {
    // - - - internal cycle
    CTX->microState = 2;
    return;
  }

  const i8 rel = (i8)CTX->imm8;
  CTX->registers.programCounter = (u16)(CTX->registers.programCounter + (u16)rel);
  cpuFinishInstruction();
}

static void stepCALL(CpuContext* CTX)
{
  /* CALL a16 / CALL cc,a16
     Sequence (when taken):
       M1: evaluate cond (no bus)
       M2: internal cycle
       M3: write PCH (SP-1)
       M4: write PCL (SP-2)
       M5: set PC = imm16
  */
  if (CTX->microState == 0)
  {
    CTX->conditionPassed = cpuEvalCond(CTX->instr->cond);
    CTX->microState      = 1;
    return;
  }

  if (!CTX->conditionPassed)
  {
    cpuFinishInstruction();
    return;
  }

  if (CTX->microState == 1)
  {
    CTX->microState = 2;
    return; 
  }

  if (CTX->microState == 2)
  {
    cpuStackWriteHi(CTX->registers.programCounter);
    CTX->microState = 3;
    return;
  }

  if (CTX->microState == 3)
  {
    // - - - push PC low
    cpuStackWriteLo(CTX->registers.programCounter);
    CTX->microState = 4;
    return;
  }

  // - - - final internal cycle / PC load
  CTX->registers.programCounter = CTX->imm16;
  cpuFinishInstruction();
}

static void stepRET(CpuContext* CTX)
{
  /* RET / RET cc
     When taken:
       M1: eval cond (no bus)
       M2: internal (some docs show this)
       M3: pop PCL (read)
       M4: pop PCH (read)
       M5: internal (for RET taken often 5 m-cycles; RET not-taken is shorter)
     We’ll implement stepped reads; final internal cycle can be refined later.
   */
  if (CTX->microState == 0)
  {
    CTX->conditionPassed = cpuEvalCond(CTX->instr->cond);
    CTX->microState      = 1;
    return;
  }

  if (!CTX->conditionPassed)
  {
    cpuFinishInstruction();
    return;
  }

  if (CTX->microState == 1)
  {
    CTX->microState = 2;
    return; 
  }

  if (CTX->microState == 2)
  {
    u8 lo = cpuStackReadLo();
    CTX->readData   = lo;
    CTX->microState = 3;
    return;
  }

  if (CTX->microState == 3)
  {
    u8 hi = cpuStackReadHi();
    CTX->readData   |= (u16)((u16)hi << 8);
    CTX->microState  = 4;
    return;
  }

  // - - - final internal cycle before commit
  CTX->registers.programCounter = (u16)CTX->readData;
  cpuFinishInstruction();
}

static void stepRETI(CpuContext* CTX)
{
  // - - - RETI: like RET, but sets IME immediately after return.
  if (CTX->microState < 4)
  {
    // - - - Reuse RET microcode but without condition.
    if (CTX->microState == 0) 
    { 
      CTX->conditionPassed = true; 
      CTX->microState = 1; 
      return; 
    }
    if (CTX->microState == 1) 
    { 
      CTX->microState = 2; 
      return; 
    }

    if (CTX->microState == 2)
    {
      u8 lo = cpuStackReadLo();
      CTX->readData   = lo;
      CTX->microState = 3;
      return;
    }

    if (CTX->microState == 3)
    {
      u8 hi = cpuStackReadHi();
      CTX->readData |= (u16)((u16)hi << 8);
      CTX->microState = 4;
      return;
    }
  }

  CTX->registers.programCounter = (u16)CTX->readData;

  // - - - IME becomes enabled immediately on RETI.
  CTX->ime          = true;
  CTX->imePending   = false;

  cpuFinishInstruction();
}

static void stepRST(CpuContext* CTX)
{
  /* RST n: push PC, then PC = param (vector)
     param is expected to be the vector address (0x00,0x08,...0x38).
     Sequence:
       M1: internal
       M2: push PCH
       M3: push PCL
       M4: set PC
  */
  if (CTX->microState == 0)
  {
    CTX->microState = 1;
    return; 
  }

  if (CTX->microState == 1)
  {
    cpuStackWriteHi(CTX->registers.programCounter);
    CTX->microState = 2;
    return;
  }

  if (CTX->microState == 2)
  {
    cpuStackWriteLo(CTX->registers.programCounter);
    CTX->microState = 3;
    return;
  }

  CTX->registers.programCounter = (u16)CTX->instr->param;
  cpuFinishInstruction();
}


// - - - Public family entry point
void opsJumpStep(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->state != CPU_STATE_EXECUTE) TODO;
  if (!ctx->instr) TODO;

  // - - -  Bump M-cycle counter for the instruction
  ctx->mCycleInInstr++;

  switch (ctx->instr->type)
  {
    case IN_JP:
      stepJP(ctx);
      return;

    case IN_JPHL:
      stepJPHL(ctx);
      return;

    case IN_JR:
      stepJR(ctx);
      return;

    case IN_CALL:
      stepCALL(ctx);
      return;

    case IN_RET:
      stepRET(ctx);
      return;

    case IN_RETI:
      stepRETI(ctx);
      return;

    case IN_RST:
      stepRST(ctx);
      return;

    default:
      TODO;
      return;
  }
}
