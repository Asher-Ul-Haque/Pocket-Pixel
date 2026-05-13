#include <cpu/instruction.h>
#include <cpu/interrupts.h>
#include <cpu/ops.h>
#include <cpu/cpu.h>

ExecStatus instrDisableInterrupt(void)
{
  CpuContext* ctx = cpuGetContext();
  ctx->ime        = false;
  ctx->imeDelay   = false;
  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrEnableInterrupt(void)
{
  CpuContext* ctx = cpuGetContext();
  ctx->imeDelay   = true;
  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrNop(void) { return EXEC_STATUS_DONE_IMMEDIATE; }

ExecStatus instrStop(void)
{
  CpuContext* ctx = cpuGetContext();
  ctx->stopped    = true;

  // - - - TODO: handle CB 
  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrHalt(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - check if an interrupt is already pending 
  bool pending = cpuInterruptPending();

  if (ctx->ime) ctx->halted = true;
  else 
  {
    if (pending)
    {
      // - - - HALT BUG: IME is 0 and interrupt is already pending, CPU fails to increment PC for the next instruction 
      ctx->haltBug = true;
    }
    else ctx->halted = true;
  }

  return EXEC_STATUS_DONE_IMMEDIATE;
}

