#include <cpu/cpu.h>
#include <bus.h>
#include <cpu/instructions.h>

/**
 * @file cpu_decode.c
 * @brief Fetch/decode only. Execution happens elsewhere.
*/

static void clearDecoded(void)
{
  CpuContext* ctx = cpuGetContext();

  ctx->readData  = 0;
  ctx->memDest   = 0;
  ctx->destIsMem = false;

  ctx->isCB                 = false;
  ctx->cbOpcode             = 0;
  ctx->currentInstruction   = NULL;
  ctx->currentOpcode        = 0;

  ctx->imm8  = 0;
  ctx->imm16 = 0;
}

void cpuFetchAndDecode(void)
{
  CpuContext* ctx = cpuGetContext();
  clearDecoded();

  u16 pc = ctx->registers.programCounter;

  // - - - HALT bug: next opcode fetch repeats the last byte (PC not advanced properly). SameBoy models this by decrementing PC when haltBug is set.
  if (ctx->haltBug)
  {
    pc--;
    ctx->haltBug = false;
  }

  ctx->currentOpcode            = busRead(pc++);
  ctx->registers.programCounter = pc;

  if (ctx->currentOpcode == 0xCB)
  {
    ctx->isCB                       = true;
    ctx->cbOpcode                   = busRead(pc++);
    ctx->registers.programCounter   = pc;
    ctx->currentInstruction         = instructionGetByCBOpcode(ctx->cbOpcode);
  }
  else
  {
    ctx->currentInstruction = instructionGetByOpcode(ctx->currentOpcode);
  }

  /* Best-effort immediate prefetch for tracing */
  if (!ctx->currentInstruction) return;

  switch (ctx->currentInstruction->mode)
  {
    case AM_R_D8    :
    case AM_D8      :
    case AM_R_A8    :
    case AM_A8_R    :
    case AM_MR_D8   :
      ctx->imm8 = busRead(ctx->registers.programCounter);
      ctx->registers.programCounter++;
      break;

    case AM_R_D16   :
    case AM_D16     :
    case AM_D16_R   :
    case AM_A16_R   :
    case AM_R_A16   :
      ctx->imm16 = busRead16(ctx->registers.programCounter);
      ctx->registers.programCounter++;
      ctx->registers.programCounter++;
      break;

    default:
      break;
  }
}
