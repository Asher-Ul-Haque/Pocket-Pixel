#include <cpu/interrupts.h>
#include <timer.h>

static TimerContext ctx;

TimerContext* timerGetContext(void)
{ return &ctx; }

static void timerIncrementTima(void)
{
  if (ctx.timaReloadPending) return;

  ctx.tima++;
  if (ctx.tima == 0)
  {
    ctx.timaReloadPending = true;
    ctx.timaReloadDelay   = 4;
  }
}

/**
 * TAC bits 0-1 define which bit of the internal counter triggers TIMA
 * Bit 9 (1024Hz), Bit 3 (16384Hz), Bit 5 (4096Hz), Bit 7 (8192Hz)
*/
static const u16 BIT_SELECT_MASK[] =
        {
                (1 << 9),
                (1 << 3),
                (1 << 5),
                (1 << 7)
        };

void timerInit(void)
{ memset(&ctx, 0, sizeof(ctx)); }

void timerStepMCycle(void)
{
  // - - - In Game Boy, the internal counter increments every T-cycle (4MHz).
  for (u8 i = 0; i < 4; i++)
  {
    if (ctx.timaReloadPending && ctx.timaReloadDelay > 0)
    {
      ctx.timaReloadDelay--;
      if (ctx.timaReloadDelay == 0)
      {
        ctx.tima              = ctx.tma;
        ctx.timaReloadPending = false;
        cpuRequestInterrupt(CPU_INT_TIMER);
      }
    }

    u16 prevCounter = ctx.internalCounter;
    ctx.internalCounter++;

    u16  mask         = BIT_SELECT_MASK[ctx.tac & 0x03];
    bool timerEnabled = (ctx.tac & 0x04) != 0;

    // - - - The condition for the timer to "tick" TIMA
    bool bitSetPrev = (timerEnabled && (prevCounter & mask));
    bool bitSetCurr = (timerEnabled && (ctx.internalCounter & mask));

    // - --  Falling edge detection
    if (bitSetPrev && !bitSetCurr)
    {
      timerIncrementTima();
    }
  }
}

u8 timerRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    case DIV_REGISTER_ADDRESS : return (u8)(ctx.internalCounter >> 8); // - - - DIV is top 8 bits
    case TIMA_REGISTER_ADDRESS: return ctx.tima;
    case TMA_REGISTER_ADDRESS : return ctx.tma;
    case TAC_REGISTER_ADDRESS : return ctx.tac | 0xF8;                 // - - - Unused bits read as 1
    default:
    FORGE_LOG_ERROR("Attempted to read from invalid timer address: 0x%04X", ADDRESS);
          FORGE_ASSERT_DEBUG(false, "Invalid timer read address");
  }
}

void timerWrite(u16 ADDRESS, u8 VALUE)
{
  // Capture old timer input signal (selected DIV bit gated by TAC enable)
  bool oldEnabled = (ctx.tac & 0x04) != 0;
  u16  oldMask    = BIT_SELECT_MASK[ctx.tac & 0x03];
  bool oldInput   = oldEnabled && ((ctx.internalCounter & oldMask) != 0);

  switch (ADDRESS)
  {
    // - - - Any write to DIV resets the entire 16-bit counter to 0.
    case DIV_REGISTER_ADDRESS:
      ctx.internalCounter = 0;
          break;

    case TIMA_REGISTER_ADDRESS:
      // Keep existing semantics: direct write cancels pending reload.
      ctx.tima              = VALUE;
          ctx.timaReloadPending = false;
          ctx.timaReloadDelay   = 0;
          return;

    case TMA_REGISTER_ADDRESS:
      ctx.tma = VALUE;
          return;

    case TAC_REGISTER_ADDRESS:
      ctx.tac = VALUE;
          break;

    default:
      return;
  }

  // Recompute input after DIV/TAC mutation.
  bool newEnabled = (ctx.tac & 0x04) != 0;
  u16  newMask    = BIT_SELECT_MASK[ctx.tac & 0x03];
  bool newInput   = newEnabled && ((ctx.internalCounter & newMask) != 0);

  // Timer glitch: increment on falling edge.
  if (oldInput && !newInput)
  {
    timerIncrementTima();
  }
}
