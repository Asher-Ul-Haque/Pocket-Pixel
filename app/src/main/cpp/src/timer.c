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

void timerTick(void) 
{
  u16 prevCounter = ctx.internalCounter;
  ctx.internalCounter++;

  // - - - Timer logic depends on TAC bit 2 (Timer Enable)
  bool timerEnabled = (ctx.tac & 0x04) != 0;
    
  if (timerEnabled) 
  {
    u16 mask = BIT_SELECT_MASK[ctx.tac & 0x03];
        
    // - - - Falling Edge Detection: If the selected bit was 1 and is now 0, increment TIMA.
    if ((prevCounter & mask) && !(ctx.internalCounter & mask)) 
    {
      ctx.tima++;

      if (ctx.tima == 0) 
      { 
        // - - - Overflow
        ctx.tima = ctx.tma;                 // - - - Reset to modulo
        cpuRequestInterrupt(CPU_INT_TIMER); // - - - Request Interrupt (Bit 2)
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
  switch (ADDRESS) 
  {
    // - - - Any write to DIV resets the entire 16-bit counter to 0.
    case DIV_REGISTER_ADDRESS : ctx.internalCounter = 0;     break;
    case TIMA_REGISTER_ADDRESS: ctx.tima            = VALUE; break;
    case TMA_REGISTER_ADDRESS : ctx.tma             = VALUE; break;
    case TAC_REGISTER_ADDRESS : ctx.tac             = VALUE; break;
  }
}
