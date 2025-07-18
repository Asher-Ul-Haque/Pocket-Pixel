#include "../include/interrupt.h"
#include "../include/stack.h"
#include "../include/cpu.h"

void interruptHandle(CPUcontext* CTX, u16 ADDRESS) 
{
  stackPush16(CTX->regs.programCounter);
  CTX->regs.programCounter = ADDRESS;
}

bool interruptCheck(CPUcontext* CTX, u16 ADDRESS, InterruptType TYPE) 
{
  if (CTX->interruptFlags & TYPE && CTX->interrupt & TYPE) 
  {
    interruptHandle(CTX, ADDRESS);
    CTX->interruptFlags         &= ~TYPE;
    CTX->halted                  = false;
    CTX->interruptMasterEnabled  = false;

    return true;
  }

  return false;
}

void cpuHandleInterrupts(CPUcontext* CTX) 
{
  if      (interruptCheck(CTX, 0x40, IT_VBLANK))    {}
  else if (interruptCheck(CTX, 0x48, IT_LCD_STAT))  {}
  else if (interruptCheck(CTX, 0x50, IT_TIMER))     {}
  else if (interruptCheck(CTX, 0x58, IT_SERIAL))    {}
  else if (interruptCheck(CTX, 0x60, IT_JOYPAD))    {} 
}
