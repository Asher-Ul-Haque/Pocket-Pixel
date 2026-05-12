#include <cpu/interrupts.h>
#include <timer.h>

static TimerContext ctx;

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
      ctx.tima++;

      // - - - Overflow 
      if (ctx.tima == 0) 
      {
        // - - - Note: Hardware actually has a 1-M-cycle delay before 
        // - - - TIMA is loaded with TMA and the interrupt is fired.
        ctx.tima = ctx.tma;
        cpuRequestInterrupt(CPU_INT_TIMER);
      }
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
  bool timerEnabled = (ctx.tac & 0x04) != 0;
  u16  mask         = BIT_SELECT_MASK[ctx.tac & 0x03];
  bool bitSetPrev   = timerEnabled && (ctx.internalCounter & mask);

  switch (ADDRESS) 
  {
    // - - - Any write to DIV resets the entire 16-bit counter to 0.
    case DIV_REGISTER_ADDRESS : ctx.internalCounter = 0;     break;
    case TIMA_REGISTER_ADDRESS: ctx.tima            = VALUE; break;
    case TMA_REGISTER_ADDRESS : ctx.tma             = VALUE; break;
    case TAC_REGISTER_ADDRESS : ctx.tac             = VALUE; break;
  }

  // - - - Check if the write caused a falling edge
  bool  timerEnabledNow = (ctx.tac & 0x04) != 0;
  u16   maskNow         = BIT_SELECT_MASK[ctx.tac & 0x03];
  bool  bitSetNow       = timerEnabledNow && (ctx.internalCounter & maskNow);

  if (bitSetPrev && !bitSetNow) 
  {
    ctx.tima++;
    if (ctx.tima == 0) 
    {
      ctx.tima = ctx.tma;
      cpuRequestInterrupt(CPU_INT_TIMER);
    }
  }
}
