#include <bus.h>
#include <cpu/cpu.h>

static CpuContext ctx;

CpuContext* cpuGetContext(void)
{
  return &ctx;
}

static inline u8 interruptQueue(void)
{
  const CpuContext* ctx = cpuGetContext();
  return (u8)(ctx->interrupt & ctx->interruptFlags & 0x1Fu);
}

bool cpuTick(void)
{
  if (ctx.enablingIme)
  {
    ctx.interruptMasterEnabled = true;
    ctx.enablingIme            = false;
  }

  const u8 queuedInterrupts = interruptQueue();

  // - - - Wake from HALT 
  if (ctx.halted && ctx.interruptMasterEnabled && queuedInterrupts)
  {
    ctx.halted = false;
    TODO_COMMENT("Implement DMA/HDMA wake side effects");
  }

  // - - - TODO: ISR call sequence and timing.
  if (!ctx.halted)
  {
    u16 pcAtFetch = ctx.registers.programCounter;
    cpuFetchAndDecode();

    #ifdef DEBUG
      char line[256];
      cpuTraceLineToString(pcAtFetch, line, sizeof(line));
      FORGE_LOG_TRACE("%s", line);
    #endif

    cpuExecDecoded();
  }
  return true;
}
