#include "../include/cpu.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/debugger.h"
#include "../include/instruction.h"
#include "../include/timer.h"
#include "../include/emu.h"

CPUcontext ctx = {0};

void cpuInit() 
{
  ctx.regs.programCounter        = 0x100;
  ctx.regs.stackPointer          = 0xFFFE;
  *((i16*)&ctx.regs.accumulator) = 0xB001;
  *((i16*)&ctx.regs.b)           = 0x1300;
  *((i16*)&ctx.regs.d)           = 0xD800;
  *((i16*)&ctx.regs.h)           = 0x4D01;
  ctx.interrupt                  = 0;
  ctx.interruptFlags             = 0;
  ctx.interruptMasterEnabled     = false;
  ctx.enablingIme                = false;
  timer_get_context()->div       = 0xABCC;
}

static void fetchInstruction() 
{
  ctx.currentOpcode      = busRead(ctx.regs.programCounter++);
  ctx.currentInstruction = instructionGetByOpcode(ctx.currentOpcode);
}

void fetchData();

static void execute() 
{
  INSTRUCTION_PROCESSOR proc = InstructionGetProcessor(ctx.currentInstruction->type);
  FORGE_ASSERT_MESSAGE(proc, "Instruction Processor cannot be null");
  proc(&ctx);
}

bool cpuStep() 
{
  if (!ctx.halted) 
  {
    u16 pc = ctx.regs.programCounter;

    fetchInstruction();
    emuCycles(1);
    fetchData();

    char flags[16];
    sprintf(flags, "%c%c%c%c", 
        ctx.regs.flags & (1 << 7) ? 'Z' : '-',
        ctx.regs.flags & (1 << 6) ? 'N' : '-',
        ctx.regs.flags & (1 << 5) ? 'H' : '-',
        ctx.regs.flags & (1 << 4) ? 'C' : '-'
    );

    char inst[16];
    instructionToStr(&ctx, inst);

    FORGE_LOG_TRACE("%08lX - %04X: %-12s (%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X", 
      emuGetContext()->ticks,
      pc, 
      inst, 
      ctx.currentOpcode,
      busRead(pc + 1), 
      busRead(pc + 2), 
      ctx.regs.accumulator, 
      flags, 
      ctx.regs.b, 
      ctx.regs.c,
      ctx.regs.d, 
      ctx.regs.e, 
      ctx.regs.h, 
      ctx.regs.l);

    if (ctx.currentInstruction == NULL) 
    {
      FORGE_LOG_FATAL("Unknown Instruction! %02X", ctx.currentOpcode);
      FORGE_ASSERT(false);
    }

    debuggerUpdate();
    debuggerPrint();

    execute();
  } 
  else 
  {
    emuCycles(1);
    if (ctx.interruptFlags) ctx.halted = false;
  }

  if (ctx.interruptMasterEnabled) 
  {
    cpuHandleInterrupts(&ctx);
    ctx.enablingIme = false;
  }

  if (ctx.enablingIme) ctx.interruptMasterEnabled = true;
  return true;
}

u8 cpuGetInterrupt() 
{ return ctx.interrupt; }

void cpuSetInterruptRegister(u8 N) 
{ ctx.interrupt = N; }

void cpuRequestInterrupt(InterruptType TYPE) 
{ ctx.interruptFlags |= TYPE; }
