#include <bus.h>
#include <cpu/cpu.h>
#include <cpu/instructions.h>
#include <stdio.h>

void cpuInstructionToString(char* OUT, u32 OUT_SIZE)
{
  CpuContext* ctx = cpuGetContext();
  if (!OUT || OUT_SIZE == 0) return;

  if (!ctx->currentInstruction)
  {
    snprintf(OUT, OUT_SIZE, "<UNKNOWN>");
    return;
  }

  const char* name = instructionGetName(ctx->currentInstruction->type);

  switch (ctx->currentInstruction->mode)
  {
    case AM_IMP:
      snprintf(OUT, OUT_SIZE, "%s", name);
      return;

    case AM_R:
      snprintf(OUT, OUT_SIZE, "%s %s", name, instructionGetRegName(ctx->currentInstruction->reg1));
      return;

    case AM_R_R:
      snprintf(OUT, OUT_SIZE, "%s %s,%s", name,
               instructionGetRegName(ctx->currentInstruction->reg1),
               instructionGetRegName(ctx->currentInstruction->reg2));
      return;

    case AM_R_D8:
    case AM_R_A8:
      snprintf(OUT, OUT_SIZE, "%s %s,$%02X", name,
               instructionGetRegName(ctx->currentInstruction->reg1), ctx->imm8);
      return;

    case AM_D8:
      snprintf(OUT, OUT_SIZE, "%s $%02X", name, ctx->imm8);
      return;

    case AM_R_D16:
      snprintf(OUT, OUT_SIZE, "%s %s,$%04X", name,
               instructionGetRegName(ctx->currentInstruction->reg1), ctx->imm16);
      return;

    case AM_D16:
      snprintf(OUT, OUT_SIZE, "%s $%04X", name, ctx->imm16);
      return;

    case AM_A16_R:
      snprintf(OUT, OUT_SIZE, "%s ($%04X),%s", name, ctx->imm16, instructionGetRegName(ctx->currentInstruction->reg1));
      return;

    case AM_R_A16:
      snprintf(OUT, OUT_SIZE, "%s %s,($%04X)", name, instructionGetRegName(ctx->currentInstruction->reg1), ctx->imm16);
      return;

    default:
      snprintf(OUT, OUT_SIZE, "%s <mode=%d>", name, (int)ctx->currentInstruction->mode);
      return;
  }
}

void cpuTraceLineToString(u16 PC_AT_FETCH, char* OUT, u32 OUT_SIZE)
{
  if (!OUT || OUT_SIZE == 0) return;

  char inst[64];
  const CpuContext* ctx = cpuGetContext();
  cpuInstructionToString(inst, sizeof(inst));

  u8 b0 = busRead(PC_AT_FETCH);
  u8 b1 = busRead((u16)(PC_AT_FETCH + 1));
  u8 b2 = busRead((u16)(PC_AT_FETCH + 2));

  snprintf(OUT, OUT_SIZE,
           "%04X: %-24s (%02X %02X %02X) A:%02X F:%02X BC:%02X%02X DE:%02X%02X HL:%02X%02X SP:%04X",
           PC_AT_FETCH, inst, b0, b1, b2,
           ctx->registers.a, ctx->registers.f,
           ctx->registers.b, ctx->registers.c,
           ctx->registers.d, ctx->registers.e,
           ctx->registers.h, ctx->registers.l,
           ctx->registers.stackPointer);
}
