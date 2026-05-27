#include <cpu/instruction.h>
#include <cpu/interrupts.h>
#include <stdio.h>
#include <utils/asserts.h>
#include <bus.h>
#include <cpu/cpu.h>
#include <cartridge/cartridge.h>

static CpuContext ctx;

CpuContext* cpuGetContext(void)
{ return &ctx; }

void cpuInit(void)
{
  CpuContext* ctx = cpuGetContext();
  memset(ctx, 0, sizeof(*ctx));
  memset(cpuInterruptGetContext(), 0, sizeof(InterruptContext));

  // - - - Model selection is read from cartridge; set post-boot registers.
  const CartContext* cart = cartridgeGetContext();
  FORGE_ASSERT_DEBUG(cart->initialized, "Must load cartridge before initializing CPU");

  // - - - Baseline DMG/CGB post-BIOS register values
  ctx->registers.programCounter  = START_VALUE_PROGRAM_COUNTER;
  ctx->registers.stackPointer    = START_VALUE_STACK_POINTER;

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

  ctx->currentInstruction = NULL;
  ctx->servicingInt       = CPUT_INT_NONE;
  ctx->doubleSpeed        = false;
  ctx->haltBug            = false;
  ctx->halted             = false;
  ctx->ime                = false;
  ctx->isCB               = false;
  ctx->currentOpcode      = 0;
  ctx->imeDelay           = 0;
  ctx->latchedAddr16      = 0;
  ctx->latchedVal8        = 0;
  ctx->pcAtFetch          = 0;
  ctx->totalMCycles       = 0;
}

/**
  * @brief Handles the 5-M-cycle interrupt service routine sequence.
  * This is hardcoded to ensure exact bus timing as per technical reference.
*/
static bool handleInterruptServiceRoutine(void)
{
  switch (ctx.mCycle)
  {
    case M2: // - - - M2: Internal delay 1
      return false;

    case M3: // - - - M3: Push PC High
      ctx.registers.stackPointer--;
      busWrite(ctx.registers.stackPointer, (u8)(ctx.registers.programCounter >> 8));
      return false;

    case M4: // - - - M4: Push PC Low 
      ctx.registers.stackPointer--;
      busWrite(ctx.registers.stackPointer, (u8)(ctx.registers.programCounter & 0xFF));
      return false;

    case M5: // - - - M5: Set PC to vector and finish
      cpuInterruptAcknowledge(ctx.servicingInt);
      ctx.registers.programCounter  = cpuInterruptVector(ctx.servicingInt);
      ctx.ime                       = false;
      return true;

    default:
      return true;
  }
}


void cpuTick(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - 1. If Halted, just wait for interrupts 
  if (ctx->halted)
  {
    ctx->totalMCycles++;
    if (cpuInterruptPending())  ctx->halted = false;
    else                        return;
  }

  // - - - 2. Handle Interrupt Sampling
  if (ctx->mCycle == M1)
  {
    if (ctx->ime && cpuInterruptPending())
    {
      ctx->servicingInt = cpuInterruptGetHighest();
      ctx->ime          = false;
      ctx->mCycle       = M2;
    }

    // - - - EI delay logic: IME Is enabled one instruction after EI
    if (ctx->imeDelay)
    {
      ctx->ime      = true;
      ctx->imeDelay = false;
    }
  }

  // - - - 3. Fetch or Execute 
  ExecStatus status = EXEC_STATUS_CONTINUE;

  // - - - Run a 5 cycle interrupt routine 
  if (ctx->servicingInt != CPUT_INT_NONE)
  {
    bool isrFinished  = handleInterruptServiceRoutine();
    status            = isrFinished ? EXEC_STATUS_DONE : EXEC_STATUS_CONTINUE;

    if (status == EXEC_STATUS_DONE)
    {
      ctx->servicingInt = CPUT_INT_NONE;
      ctx->mCycle       = M1;
    }
    else ctx->mCycle++;
  }

  else if (ctx->mCycle == M1)
  {
    // - - - M1: Fetch opcode 
    ctx->pcAtFetch = ctx->registers.programCounter;

    // - - - Handle HALT bug: PC does not increment if latch is set
    u8 opcodeByte = busRead(ctx->registers.programCounter);
    if (!ctx->haltBug) ctx->registers.programCounter++;
    else               ctx->haltBug = false;

    /**
      * CB contract:
      * M1 fetches 0xCB prefix only 
      * M2 fetches / dispatches the CB opcode and runs first handler stage
    */
    if (!ctx->isCB && opcodeByte == OP_CB_PREFIX)
    {
      ctx->isCB               = true;
      ctx->currentInstruction = NULL;
      ctx->mCycle             = M2;
      ctx->totalMCycles++;
      return;
    }

    ctx->currentOpcode = opcodeByte;

    // - - - Lookup metadata
    ctx->currentInstruction = ctx->isCB ?
    instructionGetByCBOpcode(ctx->currentOpcode) :
    instructionGetByOpcode(ctx->currentOpcode);

    // - - - Transition and run the first m cycle of the instruction
    status = ctx->currentInstruction->handler();

    // - - - single m cycle instruction
    if (status == EXEC_STATUS_DONE_IMMEDIATE)
    {
      ctx->mCycle = M1;
      ctx->isCB   = false;
    }
    else
    {
      ctx->mCycle = M2;
    }
  }

  // - - - Multicycle execution phase 
  else
  {
    // - - - CB second byte fetch and first execution stage
    // - - - BUG: You might be wondering why stay in M2, reason being that I made CB handlers to expect M2, and I can either change every single one of them, or I can just stay in M2 here.

    if (ctx->isCB && ctx->currentInstruction == NULL && ctx->mCycle == M2)
    {
      u8 cbOpcode             = busRead(ctx->registers.programCounter++);
      ctx->currentOpcode      = (Opcode) (cbOpcode + CB_OFFSET);
      ctx->currentInstruction = instructionGetByCBOpcode(ctx->currentOpcode);
      status                  = ctx->currentInstruction->handler();
    }
    else status = ctx->currentInstruction->handler();

    if (status == EXEC_STATUS_DONE)
    {
      ctx->mCycle             = M1;
      ctx->isCB               = false;
      ctx->currentInstruction = NULL;
    }
    else ctx->mCycle++;
  }

  ctx->totalMCycles++;
}

void cpuTraceLineToString(char* OUT, u32 OUT_SIZE)
{
  CpuContext* ctx = cpuGetContext();
  u8 f = ctx->registers.f;

  char flags[5] = {
    (f & FLAG_Z) ? 'Z' : '-',
    (f & FLAG_N) ? 'N' : '-',
    (f & FLAG_H) ? 'H' : '-',
    (f & FLAG_C) ? 'C' : '-',
    '\0'
  };

  const char* mnemonic = instructionGetName(ctx->currentOpcode);
  u16 pc = ctx->pcAtFetch;
  
  // Peek at the bus to see what the instruction arguments are
  // This doesn't increment the PC, just reads for the log.
  u8 b1 = busRead(pc + 1);
  u8 b2 = busRead(pc + 2);

  snprintf(OUT, OUT_SIZE,
    "A:%02X F:%s BC:%04X DE:%04X HL:%04X SP:%04X PC:%04X | Op:%02X %02X %02X (%s)",
    ctx->registers.a,
    flags,
    (u16)((ctx->registers.b << 8) | ctx->registers.c),
    (u16)((ctx->registers.d << 8) | ctx->registers.e),
    (u16)((ctx->registers.h << 8) | ctx->registers.l),
    ctx->registers.stackPointer,
    pc,
    ctx->currentOpcode, b1, b2,
    mnemonic ? mnemonic : "UNK"
  );
}
