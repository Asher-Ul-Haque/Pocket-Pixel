#include "../include/interrupt.h"
#include "../include/stack.h"


void intHandle(CPUContext* CTX, u16 ADDRESS)
{
  stackPush16(CTX->registerFile.programCounter);
  CTX->registerFile.programCounter = ADDRESS;
}

FORGE_INLINE bool interruptCheck(CPUContext* CTX, u16 ADDRESS, InterruptType TYPE)
{
  if (CTX->interruptFlags & TYPE && CTX->interrupt & TYPE)
  {
    intHandle(CTX, ADDRESS);
    CTX->interruptFlags        &= ~INTRPT_VBLANK;
    CTX->halted                 = false;
    CTX->interruptMasterEnabled = false;

    return true;
  }
  return false;
}

void cpuHandleInterrupts(CPUContext* CTX)
{
  if      (interruptCheck(CTX, 0x40,INTRPT_VBLANK))
  {}
  else if (interruptCheck(CTX, 0x48, INTRPT_LCD_STAT))
  {}
  else if (interruptCheck(CTX, 0x50, INTRPT_TIMER))
  {}
  else if (interruptCheck(CTX, 0x58, INTRPT_SERIAL))
  {}
  else if (interruptCheck(CTX, 0x60, INTRPT_JOYPAD))
  {}
}

void cpuRequestInterrupt(InterruptType TYPE)
{
}
