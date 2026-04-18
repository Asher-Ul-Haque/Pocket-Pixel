#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <stdio.h>
#include <string.h>

/**
 * @file trace.c
 * @brief Trace and disassembly formatting helpers.
 *
 * These functions are for debugging/logging only and must not mutate CPU state.
*/

// - - - Small helper: safe append into OUT buffer.
static void appendf(char* OUT, u32 OUT_SIZE, u32* IDX, const char* FORMAT, ...)
{
  if (!OUT || OUT_SIZE == 0)    return;
  if (!IDX)                     return;

  if (*IDX >= OUT_SIZE) return;

  va_list args;
  va_start(args, FORMAT);

  const i32 remaining = (i32)(OUT_SIZE - *IDX);
  if (remaining > 0)
  {
    i32 wrote = vsnprintf(OUT + *IDX, (u64)remaining, FORMAT, args);
    if (wrote < 0) wrote = 0;

    // - - - Clamp: vsnprintf returns chars that *would* have been written 
    if ((u32)wrote >= (u32)remaining) *IDX = OUT_SIZE - 1;
    else                              *IDX += (u32)wrote;
  }

  va_end(args);

  OUT[OUT_SIZE - 1] = '\0';
}

static const char* condToString(ConditionType CONDITION)
{
  switch (CONDITION)
  {
    case CT_NONE: return "";
    case CT_NZ  : return "NZ";
    case CT_Z   : return "Z";
    case CT_NC  : return "NC";
    case CT_C   : return "C";
    default:      return "?";
  }
}

static void formatOperandText(const CpuContext* CTX, char* OUT, u32 OUT_SIZE)
{
  // - - - This produces the operand string portion only (no mnemonic).
  if (!OUT || OUT_SIZE == 0) return;
  OUT[0] = '\0';

  if (!CTX || !CTX->instr) return;

  const Instruction* ins = CTX->instr;

  char r1[8] = {0};
  char r2[8] = {0};

  const char* r1s = instructionGetRegName(ins->reg1);
  const char* r2s = instructionGetRegName(ins->reg2);

  // - - - Guard: instructionGetRegName can return ""
  snprintf(r1, sizeof(r1), "%s", r1s ? r1s : "");
  snprintf(r2, sizeof(r2), "%s", r2s ? r2s : "");

  switch (ins->mode)
  {
    case AM_IMP:
      return;

    case AM_R:
      snprintf(OUT, (u64)OUT_SIZE, "%s", r1);
      return;

    case AM_R_R:
      snprintf(OUT, (u64)OUT_SIZE, "%s,%s", r1, r2);
      return;

    // - - - Used by JR e8 and other immediate-only ops. Prefer signed for JR-ish.
    case AM_D8:
      snprintf(OUT, (u64)OUT_SIZE, "$%02X", (unsigned)CTX->imm8);
      return;

    case AM_D16:
      snprintf(OUT, (u64)OUT_SIZE, "$%04X", (unsigned)CTX->imm16);
      return;

    case AM_R_D8:
      snprintf(OUT, (u64)OUT_SIZE, "%s,$%02X", r1, (unsigned)CTX->imm8);
      return;

    case AM_R_D16:
      snprintf(OUT, (u64)OUT_SIZE, "%s,$%04X", r1, (unsigned)CTX->imm16);
      return;

    // - - - rr, r
    case AM_MR_R:
      snprintf(OUT, (u64)OUT_SIZE, "(%s),%s", r1, r2);
      return;

    // - - - r, rr
    case AM_R_MR:
      snprintf(OUT, (u64)OUT_SIZE, "%s,(%s)", r1, r2);
      return;

    // - - - HL, d8 style: reg1 holds the pair, usually HL
    case AM_MR_D8:
      snprintf(OUT, (u64)OUT_SIZE, "(%s),$%02X", r1, (unsigned)CTX->imm8);
      return;

    case AM_A16_R:
      snprintf(OUT, (u64)OUT_SIZE, "($%04X),%s", (unsigned)CTX->imm16, r1);
      return;

    case AM_R_A16:
      snprintf(OUT, (u64)OUT_SIZE, "%s,($%04X)", r1, (unsigned)CTX->imm16);
      return;

    case AM_A8_R:
      snprintf(OUT, (u64)OUT_SIZE, "($FF00+$%02X),%s", (unsigned)CTX->imm8, r1);
      return;

    case AM_R_A8:
      snprintf(OUT, (u64)OUT_SIZE, "%s,($FF00+$%02X)", r1, (unsigned)CTX->imm8);
      return;

    case AM_MR_C:
      // - - - Typically (0xFF00 + C), A 
      snprintf(OUT, (u64)OUT_SIZE, "($FF00+C),%s", r1);
      return;

    case AM_R_MR_C:
      // - - - Typically A, (0xFF00 + C)
      snprintf(OUT, (u64)OUT_SIZE, "%s,($FF00+C)", r1);
      return;

    default:
      snprintf(OUT, (u64)OUT_SIZE, "<?>");
      return;
  }
}

void cpuInstructionToString(char* OUT, u32 OUT_SIZE)
{
  if (!OUT || OUT_SIZE == 0) return;
  OUT[0] = '\0';

  CpuContext* ctx = cpuGetContext();
  if (!ctx || !ctx->instr)
  {
    snprintf(OUT, (u64)OUT_SIZE, "<no-instr>");
    OUT[OUT_SIZE - 1] = '\0';
    return;
  }

  const Instruction* ins    = ctx->instr;
  const char*        name   = instructionGetName(ins->type);
  if (!name)         name   = "UNK";

  char ops[64];
  formatOperandText(ctx, ops, sizeof(ops));

  // - - - Condition formatting for control flow mnemonics (JP/JR/CALL/RET).
  const char* cond = condToString(ins->cond);

  if (cond && cond[0] != '\0')
  {
    if (ops[0] != '\0') snprintf(OUT, (u64)OUT_SIZE, "%s %s,%s", name, cond, ops);
    else                snprintf(OUT, (u64)OUT_SIZE, "%s %s", name, cond);
  }
  else
  {
    if (ops[0] != '\0') snprintf(OUT, (u64)OUT_SIZE, "%s %s", name, ops);
    else                snprintf(OUT, (u64)OUT_SIZE, "%s", name);
  }

  OUT[OUT_SIZE - 1] = '\0';
}

void cpuTraceLineToString(u16 PC_AT_FETCH, char* OUT, u32 OUT_SIZE)
{
  if (!OUT || OUT_SIZE == 0) return;
  OUT[0] = '\0';

  const CpuContext* ctx = cpuGetContext();
  if (!ctx)
  {
    snprintf(OUT, (u64)OUT_SIZE, "<no-cpu>");
    OUT[OUT_SIZE - 1] = '\0';
    return;
  }

  // - - - Instruction bytes (best-effort).We only have opcode + maybe cbOpcode + imm8/imm16; we do not re-read bus here.
  u8 b0 = ctx->opcode;
  u8 b1 = 0;
  u8 b2 = 0;

  if (ctx->isCB)
  {
    b0 = 0xCB;
    b1 = ctx->cbOpcode;
  }

  if (ctx->instr)
  {
    // - - - Fill b1/b2 from immediates if applicable. This is approximate but stable for trace.
    switch (ctx->instr->mode)
    {
      case AM_D8   :
      case AM_R_D8 :
      case AM_MR_D8:
      case AM_A8_R :
      case AM_R_A8 :
        if (ctx->isCB) { b2 = ctx->imm8; }
        else           { b1 = ctx->imm8; }
        break;

      case AM_D16  :
      case AM_R_D16:
      case AM_A16_R:
      case AM_R_A16:
        if (ctx->isCB)
        {
          b2 = (u8)(ctx->imm16 & 0xFFu); // - - - still only 3 bytes shown; CB+d16 is rare anyway
        }
        else
        {
          b1 = (u8)(ctx->imm16 & 0xFFu);
          b2 = (u8)((ctx->imm16 >> 8) & 0xFFu);
        }
        break;

      default:
        break;
    }
  }

  char instr[96];
  cpuInstructionToString(instr, sizeof(instr));

  const RegisterFile* r = &ctx->registers;

  // - - - Flags: show Z N H C as 0/1 
  const i32 z = cpuFlagZ(r) ? 1 : 0;
  const i32 n = cpuFlagN(r) ? 1 : 0;
  const i32 h = cpuFlagH(r) ? 1 : 0;
  const i32 c = cpuFlagC(r) ? 1 : 0;

  u32 idx = 0;

  // - - - Similar-ish to SameBoy style, but keep it simple and stable.
  appendf(OUT, OUT_SIZE, &idx,
          "%04X  %02X %02X %02X  %-20s  "
          "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X  "
          "SP:%04X PC:%04X  "
          "Z:%d N:%d H:%d C:%d",
          (unsigned)PC_AT_FETCH,
          (unsigned)b0, (unsigned)b1, (unsigned)b2,
          instr,
          (unsigned)r->a, (unsigned)r->f,
          (unsigned)r->b, (unsigned)r->c,
          (unsigned)r->d, (unsigned)r->e,
          (unsigned)r->h, (unsigned)r->l,
          (unsigned)r->stackPointer,
          (unsigned)r->programCounter,
          z, n, h, c);
}
