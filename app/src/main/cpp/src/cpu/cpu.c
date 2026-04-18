#include "cpu/interrupts.h"
#include "utils/asserts.h"
#include <bus.h>
#include <cpu/cpu.h>
#include <io/cartridge.h>

static CpuContext ctx;

CpuContext* cpuGetContext(void)
{
  return &ctx;
}

static void cpuClearInstructionTransient(CpuContext* CTX)
{
  CTX->pcAtFetch        = 0;
  CTX->opcode           = 0;
  CTX->isCB             = false;
  CTX->cbOpcode         = 0;

  CTX->instr             = NULL;

  CTX->imm8             = 0;
  CTX->imm16            = 0;

  CTX->mCycleInInstr    = 0;
  CTX->microState       = 0;

  CTX->addr             = 0;
  CTX->readData         = 0;
  CTX->hasAddr          = false;
  CTX->hasReadData      = false;

  CTX->conditionPassed  = false;
}

void cpuReset(void)
{
  CpuContext* ctx = cpuGetContext();

  RegisterFile regs = ctx->registers;
  bool doubleSpeed  = ctx->doubleSpeed;

  memset(ctx, 0, sizeof(*ctx));

  ctx->registers   = regs;
  ctx->doubleSpeed = doubleSpeed;

  ctx->halted      = false;
  ctx->stopped     = false;
  ctx->haltBug     = false;

  ctx->ime         = false;
  ctx->imePending  = false;

  ctx->mCyclesTotal = 0;
  ctx->state        = CPU_STATE_FETCH;

  cpuClearInstructionTransient(ctx);
}

void cpuInit(void)
{
  CpuContext* ctx = cpuGetContext();
  memset(ctx, 0, sizeof(*ctx));

  // - - - Model selection is read from cartridge; set post-boot registers.
  const CartContext* cart = cartridgeGetContext();
  FORGE_ASSERT_DEBUG(cart->initialized, "Must load cartridge before initializing CPU");

  // - - - Baseline DMG/CGB post-BIOS register values
  ctx->registers.programCounter = START_VALUE_PROGRAM_COUNTER;
  ctx->registers.stackPointer   = START_VALUE_STACK_POINTER;

  if (cart->mode == MODE_DMG_GAMEBOY)
  {
    cpuSetAF(&ctx->registers, START_VALUE_AF_DMG);
    cpuSetBC(&ctx->registers, START_VALUE_BC_DMG);
    cpuSetDE(&ctx->registers, START_VALUE_DE_DMG);
    cpuSetHL(&ctx->registers, START_VALUE_HL_DMG);
  }
  else
  {
    cpuSetAF(&ctx->registers, START_VALUE_AF_CGB);
    cpuSetBC(&ctx->registers, START_VALUE_BC_CGB);
    cpuSetDE(&ctx->registers, START_VALUE_DE_CGB);
    cpuSetHL(&ctx->registers, START_VALUE_HL_CGB);
  }

  ctx->doubleSpeed = false;

  ctx->halted  = false;
  ctx->stopped = false;
  ctx->haltBug = false;

  ctx->ime        = false;
  ctx->imePending = false;

  ctx->mCyclesTotal = 0;
  ctx->state        = CPU_STATE_FETCH;

  cpuClearInstructionTransient(ctx);
}


void cpuTickMCycle(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - STOP is special; defer for now.
  if (ctx->stopped)
  {
    TODO_COMMENT("Implement STOP behavior (entering low-power mode, waiting for button press or serial input, etc.)");
  }

  // - - - HALT: for now, just spin until interrupt logic exists.
  if (ctx->halted)
  {
    TODO_COMMENT("Implement HALT behavior (CPU stops executing instructions until an interrupt occurs, but still responds to interrupts and can wake from HALT)");
  }

  switch (ctx->state)
  {
    case CPU_STATE_FETCH   :
    case CPU_STATE_FETCH_CB:
    case CPU_STATE_DECODE  :
      cpuDecodeStep();
      break;

    case CPU_STATE_EXECUTE:
      cpuExecuteStep();
      break;

    case CPU_STATE_INTERRUPT_ENTRY:
      cpuInterruptEntryStep();
      break;

    default:
      break;
  }

  ctx->mCyclesTotal++;
}

void cpuTick(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - -  If we’re already at FETCH, we want to run exactly one instruction, meaning: leave FETCH, then come back to FETCH.
  bool leftFetch = false;

  for (;;)
  {
    cpuTickMCycle();

    if      (ctx->state != CPU_STATE_FETCH) leftFetch = true;
    else if (leftFetch)                     break;
  }
}
