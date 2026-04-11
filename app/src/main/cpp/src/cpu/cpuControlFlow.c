#include <bus.h>
#include <cpu/cpu.h>
#include <cpu/cpuOpcodeHandlers.h>
#include <cpu/instructions.h>
#include <cpu/registers.h>

static inline bool conditionMet(ConditionType COND)
{
  CpuContext* CTX = cpuGetContext();
  switch (COND)
  {
    case CT_NONE : return true;
    case CT_Z    : return CPU_FLAG_ZERO_GET;
    case CT_NZ   : return !CPU_FLAG_ZERO_GET;
    case CT_C    : return CPU_FLAG_CARRY_GET;
    case CT_NC   : return !CPU_FLAG_CARRY_GET;
    default:
      FORGE_LOG_ERROR("[CPU] : Invalid condition type %d", COND);
      return false;
  }
}

static inline void push16(u16 VALUE)
{
  CpuContext* ctx = cpuGetContext();
  ctx->registers.stackPointer--;
  busWrite(ctx->registers.stackPointer, (u8)(VALUE >> 8));
  ctx->registers.stackPointer--;
  busWrite(ctx->registers.stackPointer, (u8)(VALUE & 0xFF));
}

static inline u16 pop16(void)
{
  CpuContext*   ctx     = cpuGetContext();
  u8            high    = busRead(ctx->registers.stackPointer++);
  u8            low     = busRead(ctx->registers.stackPointer++);
  return (u16)((high << 8) | low);
}

void opJP_a16(u8 OPCODE) { (void)OPCODE; cpuGetContext()->registers.programCounter = cpuGetContext()->imm16; }
void opJP_HL(u8 OPCODE)  { (void)OPCODE; cpuGetContext()->registers.programCounter = (u16)((cpuGetContext()->registers.h << 8) | cpuGetContext()->registers.l); }

void opJR_r8(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  ctx->registers.programCounter = (u16)(ctx->registers.programCounter + (i8)ctx->imm8);
}

void opJR_cc_r8(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  if (conditionMet(ctx->currentInstruction->cond))
  {
    ctx->registers.programCounter = (u16)(ctx->registers.programCounter + (i8)ctx->imm8);
  }
}

void opCALL_a16(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  push16(ctx->registers.programCounter);
  ctx->registers.programCounter = ctx->imm16;
}

void opCALL_cc_a16(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  if (conditionMet(ctx->currentInstruction->cond))
  {
    push16(ctx->registers.programCounter);
    ctx->registers.programCounter = ctx->imm16;
  }
}

void opRET(u8 OPCODE)
{
  (void)OPCODE;
  cpuGetContext()->registers.programCounter = pop16();
}

void opRET_cc(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  if (conditionMet(ctx->currentInstruction->cond))
  {
    ctx->registers.programCounter = pop16();
  }
}

void opRETI(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx               = cpuGetContext();
  ctx->registers.programCounter = pop16();
  ctx->interruptMasterEnabled   = true;
}

void opRST(u8 OPCODE)
{
  CpuContext* ctx = cpuGetContext();
  // - - - RST target is encoded in opcode: 0xC7,0xCF,... -> 0x00,0x08
  u16 target = (u16)(OPCODE & 0x38);
  push16(ctx->registers.programCounter);
  ctx->registers.programCounter = target;
}


// - - - Opcode Handlers : CPU control - - - 

void opDI(u8 OPCODE)   { (void)OPCODE; cpuGetContext()->interruptMasterEnabled = false; }

void opEI(u8 OPCODE)   { (void)OPCODE; cpuGetContext()->enablingIme = true; }

void opHALT(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  ctx->halted     = true;
}

void opSTOP(u8 OPCODE)
{
  (void)OPCODE;
  CpuContext* ctx = cpuGetContext();
  ctx->stopped    = true;
  /* TODO: CGB speed switch via KEY1 once IO exists */
}
