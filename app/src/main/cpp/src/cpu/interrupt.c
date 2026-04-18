#include <cpu/interrupts.h>
#include <cpu/cpu.h>
#include <bus.h>

/**
 * @file interrupt.c
 * @brief Interrupt pending detection and interrupt entry microsequence.
 *
 * This is intentionally separated so:
 * - HALT behavior can query "any pending" without embedding IO details in ops files
 * - interrupt entry sequence can be cycle-stepped cleanly
 *
 * IMPORTANT:
 * This file needs access to IE (0xFFFF) and IF (0xFF0F).
 * For now, we read/write them via busRead/busWrite. Later you can route through IO helpers.
*/

static InterruptContext ctx;
InterruptContext* cpuInterruptGetContext(void) 
{ return &ctx; }

bool cpuInterruptAnyPending(void)
{
  return (u8)(ctx.interruptEnable & ctx.interruptFlag & 0x1Fu) != 0;
}

bool cpuInterruptGetPending(CpuInterrupt* OUT_INT)
{
  const u8 pending = (u8)(ctx.interruptEnable & ctx.interruptFlag & 0x1Fu);
  if (pending == 0) return false;

  // - - - Priority order: 0..4
  for (u8 bit = 0; bit < 5; bit++)
  {
    if ((pending & (u8)(1u << bit)) != 0)
    {
      if (OUT_INT) *OUT_INT = (CpuInterrupt)bit;
      return true;
    }
  }

  // - - - should be unreachable
  return false;
}

void cpuInterruptAcknowledge(CpuInterrupt INT)
{
  u8 ir = ctx.interruptFlag;
  ir    = (u8)(ir & (u8)~(1u << (u8)INT));
  ctx.interruptFlag = ir;
}

u16 cpuInterruptVector(CpuInterrupt INT)
{
  switch (INT)
  {
    case CPU_INT_VBLANK : return CPU_INT_VEC_VBLANK;
    case CPU_INT_LCD    : return CPU_INT_VEC_LCD;
    case CPU_INT_TIMER  : return CPU_INT_VEC_TIMER;
    case CPU_INT_SERIAL : return CPU_INT_VEC_SERIAL;
    case CPU_INT_JOYPAD : return CPU_INT_VEC_JOYPAD;
    default: return 0;
  }
}

void cpuInterruptEntryStep(void)
{
  CpuContext* ctx = cpuGetContext();

  FORGE_ASSERT_DEBUG(ctx->state == CPU_STATE_INTERRUPT_ENTRY, "Must be in interrupt entry");
  
  // - - - We store which interrupt we’re servicing in ctx->readData low byte to avoid adding fields.
  if (ctx->microState == 0)
  {
    CpuInterrupt intr;
    if (!cpuInterruptGetPending(&intr))
    {
      // - - - Nothing pending; go back to fetch.
      cpuFinishInstruction();
      return;
    }

    ctx->readData = (u16)(u8)intr;

    // - - - Entering an interrupt disables IME immediately.
    ctx->ime        = false;
    ctx->imePending = false;

    // - - - First internal cycle. 
    ctx->microState = 1;
    return;
  }

  // - - - Internal cycle (M2) 
  if (ctx->microState == 1)
  {
    ctx->microState = 2;
    return;
  }

  // - - - Push PC high (M3)
  if (ctx->microState == 2)
  {
    cpuStackWriteHi(ctx->registers.programCounter);
    ctx->microState = 3;
    return;
  }

  // - - - Push PC low (M4)
  if (ctx->microState == 3)
  {
    cpuStackWriteLo(ctx->registers.programCounter);
    ctx->microState = 4;
    return;
  }

  // - - - Acknowledge IF + set PC to vector (M5-ish)
  if (ctx->microState == 4)
  {
    const CpuInterrupt intr = (CpuInterrupt)(u8)ctx->readData;

    cpuInterruptAcknowledge(intr);
    ctx->registers.programCounter = cpuInterruptVector(intr);

    // - - - Done: start fetching at vector
    ctx->microState     = 0;
    ctx->mCycleInInstr  = 0;
    ctx->state          = CPU_STATE_FETCH;
    return;
  }
}

void cpuRequestInterrupt(CpuInterrupt INT)
{
  u8 ir = ctx.interruptFlag;
  ir    = (u8)(ir | (1u << (u8)INT));
  ctx.interruptFlag = ir;
}

u8 cpuReadInterrupt(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    case ADDR_IF: return ctx.interruptFlag;
    case ADDR_IE: return ctx.interruptEnable;
    default:
      FORGE_LOG_ERROR("Attempted to read from invalid interrupt address: 0x%04X", ADDRESS);
      FORGE_ASSERT_DEBUG(false, "Invalid interrupt read address");
  }
}

void cpuWriteInterrupt(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    case ADDR_IF: ctx.interruptFlag   = VALUE; break;
    case ADDR_IE: ctx.interruptEnable = VALUE; break;
    default:
      FORGE_LOG_ERROR("Attempted to write to invalid interrupt address: 0x%04X", ADDRESS);
      FORGE_ASSERT_DEBUG(false, "Invalid interrupt write address");
  }
}
