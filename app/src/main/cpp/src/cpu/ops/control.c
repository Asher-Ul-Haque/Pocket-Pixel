#include <cpu/interrupts.h>
#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <cpu/instruction.h>

/**
 * @file control.c
 * @brief Control/system instruction family (M-cycle stepping).
 *
 * Covers:
 * - NOP
 * - DI / EI (EI uses imePending)
 * - HALT / STOP (TODO for now)
 * - Flag-control / misc single-byte ops (DAA/CPL/SCF/CCF) (TODO for now)
 *
 * Contract:
 * - Called only while ctx->state == CPU_STATE_EXECUTE
 * - Must do at most ONE M-cycle worth of work per call
 * - When instruction is complete, call cpuFinishInstruction()
*/

static void execNOP(CpuContext* CTX)
{
  // - - - NOP is 1 M-cycle 
  (void)CTX;
  cpuFinishInstruction();
}

static void execDI(CpuContext* CTX)
{
  // - - - DI: IME=0 immediately; also clear pending enable
  CTX->ime          = false;
  CTX->imePending   = false;
  cpuFinishInstruction();
}

static void execEI(CpuContext* CTX)
{
  // - - - EI: IME becomes 1 after next instruction completes
  CTX->imePending = true;
  cpuFinishInstruction();
}

static void execCPL(CpuContext* CTX)
{
  CTX->registers.a = ~CTX->registers.a;
  cpuSetN(&CTX->registers, true);
  cpuSetH(&CTX->registers, true);
  cpuFinishInstruction();
}

static void execSCF(CpuContext* CTX)
{
  cpuSetN(&CTX->registers, false);
  cpuSetH(&CTX->registers, false);
  cpuSetC(&CTX->registers, true);
  cpuFinishInstruction();
}

static void execCCF(CpuContext* CTX)
{
  cpuSetN(&CTX->registers, false);
  cpuSetH(&CTX->registers, false);
  cpuSetC(&CTX->registers, !cpuFlagC(&CTX->registers));
  cpuFinishInstruction();
}

static void execHALT(CpuContext* CTX)
{
  /* HALT behavior summary:
     - If no interrupts are pending: CPU enters halted state and stops fetching/executing.
     - If an interrupt is pending:
         - If IME=1: CPU will service interrupt (not truly halted).
         - If IME=0: HALT bug triggers: CPU continues but next opcode fetch is glitched.
  */
  if (cpuInterruptAnyPending())
  {
    if (!CTX->ime)
    {
      // - - - HALT bug case
      CTX->haltBug = true;
    }
    /* If IME=1, just let the main loop enter interrupt entry at boundary (FETCH).
       HALT itself still "completes" as an instruction. */
    cpuFinishInstruction();
    return;
  }

  // - - - No pending interrupts: enter HALT state
  CTX->halted = true;
  cpuFinishInstruction();
}

static void execSTOP(CpuContext* CTX)
{
  (void)CTX;
  CTX->stopped = true;
  // - - - STOP behavior depends on KEY1 / speed switch / joypad and CGB mode. Implement later with IO integration.
  TODO;
}

static void execDAA(CpuContext* CTX)
{
  u8   a          = CTX->registers.a;
  u8   correction = 0;
  bool setC       = false;

  // - - - Check if adjustment is needed for the lower nibble 
  if (cpuFlagH(&CTX->registers) || (!cpuFlagN(&CTX->registers) && (a & 0xF) > 9)) 
  {
    correction |= 0x06;
  }

  // - - - Check if adjustment is needed for the upper nibble
  if (cpuFlagC(&CTX->registers) || (!cpuFlagN(&CTX->registers) && a > 0x99)) 
  {
    correction |= 0x60;
    setC        = true;
  }

  // - - - Apply correction based on whether the last op was addition or subtraction
  if (cpuFlagN(&CTX->registers))  CTX->registers.a -= correction;
  else                            CTX->registers.a += correction;

  cpuSetZ(&CTX->registers, CTX->registers.a == 0);
  cpuSetH(&CTX->registers, false);
  cpuSetC(&CTX->registers, setC);
  cpuFinishInstruction();
}

void opsControlStep(void)
{
  CpuContext* ctx = cpuGetContext();

  FORGE_ASSERT_DEBUG(ctx->state == CPU_STATE_EXECUTE, "must be in execute state");
  FORGE_ASSERT_DEBUG(ctx->instr, "must have an instruction to execute");

  /* These are all single-byte instructions; treat as 1 M-cycle for now.
     When we do strict timing, we can use ctx->mCycleInInstr and ctx->instr->mCycles. */
  switch (ctx->instr->type)
  {
    case IN_NOP: execNOP(ctx);  break;
    case IN_DI : execDI(ctx);   break;
    case IN_EI : execEI(ctx);   break;

    case IN_HALT: execHALT(ctx); break;
    case IN_STOP: execSTOP(ctx); break;

    case IN_DAA: execDAA(ctx); break;
    case IN_CPL: execCPL(ctx); break;
    case IN_SCF: execSCF(ctx); break;
    case IN_CCF: execCCF(ctx); break;

    default:
      FORGE_ASSERT_DEBUG(false, "opsControlStep should only be used for control types");
      break;
  }
}
