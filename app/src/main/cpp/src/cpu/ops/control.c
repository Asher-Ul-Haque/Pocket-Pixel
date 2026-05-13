#include <cpu/cpu.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>
#include <cpu/ops.h>
#include <bus.h>

static bool cpuCheckCondition(u8 OPCODE) 
{
  CpuContext* ctx       = cpuGetContext();
  u8          condition = (OPCODE >> 3) & 0x03; // - - - Extracts bits 3 and 4

  switch (condition) 
  {
    case 0: return !(ctx->registers.f & FLAG_Z); // - - - NZ
    case 1: return  (ctx->registers.f & FLAG_Z); // - - - Z
    case 2: return !(ctx->registers.f & FLAG_C); // - - - NC
    case 3: return  (ctx->registers.f & FLAG_C); // - - - C
  }
  return false;
}

ExecStatus instrJump16BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Read the LSB of address into internal Z latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Read the MSB of address into internal W latch 
  if (ctx->mCycle == M3)
  {
    // - - - store full 16 bit target in the latch 
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.programCounter++);

    ctx->latchedAddr16 = ((u16)msb << 8) | lsb;
    return EXEC_STATUS_CONTINUE;
  }

  // - --  M4: update PC with target adress WZ 
  if (ctx->mCycle == M4)
  {
    ctx->registers.programCounter = ctx->latchedAddr16;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrJumpHL(void)
{
  CpuContext* ctx = cpuGetContext();
  ctx->registers.programCounter = cpuGetReg16(RT_HL);
  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrJumpConditional16BitImm(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: ALways read lsb into internal Z latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: ALways read MSB into internal W latch 
  if (ctx->mCycle == M3)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 = (u16)((msb << 8) | lsb);

    // - - - condition check, if false done in 3 cycles 
    if (!cpuCheckCondition(instr->opcode)) return EXEC_STATUS_DONE;
    
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Update PC, (only reached if condition is true)
  if (ctx->mCycle == M4)
  {
    ctx->registers.programCounter = ctx->latchedAddr16;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrJumpRelSigned8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Read the signed displacement 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return false;
  }

  // - - - M3: Internal ALU cycle - calculate PC + offset 
  if (ctx->mCycle == M3)
  {
    // - - - the offset is signed (-128, to 127)
    i8 offset = (i8) ctx->latchedVal8;
    ctx->registers.programCounter += offset;

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrJumpRelConditionalSigned8BitImm(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Always read the signed 8 bit displacement 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);

    // - - - if condition is false, we are done in 2 cycles 
    if (!cpuCheckCondition(instr->opcode)) return EXEC_STATUS_DONE;
    return EXEC_STATUS_CONTINUE;
  }

  // - - -M3: Internal ALU cycle - calulcate PC + offset 
  if (ctx->mCycle == M3)
  {
    i8 offset = (i8) ctx->latchedVal8;
    ctx->registers.programCounter += offset;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrCall16BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - -M2: Read lsb of target addres (nn)
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Read MSB of target address (nn)
  if (ctx->mCycle == M3)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 = (u16) ((msb << 8) | lsb);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Internal cycle (stack pointer decrement)
  if (ctx->mCycle == M4) return EXEC_STATUS_CONTINUE;

  // - - - M5: Push high byte of return addres (PC)
  if (ctx->mCycle == M5)
  {
    ctx->registers.stackPointer--;
    u8 pch = (u8) ((ctx->registers.programCounter & 0xFF00) >> 8);
    busWrite(ctx->registers.stackPointer, pch);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M6: Push low byte of return address (PC) and update PC to target 
  if (ctx->mCycle == M6)
  {
    ctx->registers.stackPointer--;
    u8 pcl = (u8) (ctx->registers.programCounter & 0x00FF);
    busWrite(ctx->registers.stackPointer, pcl);

    // - - - Fingalize jump to the address we latched in M3 
    ctx->registers.programCounter = ctx->latchedAddr16;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_DONE;
}

ExecStatus instrCallConditional16BitImm(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Always read LSB of target 
  if (ctx->mCycle == M2) 
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Alwys read MSB of target address 
  if (ctx->mCycle == M3)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 = (u16) ((msb << 8) | lsb);

    // - - - Check condition, if false, done in 3 cycles 
    if (!cpuCheckCondition(instr->opcode)) return true;

    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Internal cycle 
  if (ctx->mCycle == M4) return EXEC_STATUS_CONTINUE;

  // - - - M5: Push high byte of return address PC 
  if (ctx->mCycle == M5)
  {
    ctx->registers.stackPointer--;
    u8 pch = (u8) ((ctx->registers.programCounter & 0xFF00) >> 8) ;
    busWrite(ctx->registers.stackPointer, pch);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M6: Push low byte of return address and update PC to target 
  if (ctx->mCycle == M6)
  {
    ctx->registers.stackPointer--;
    u8 pcl = (u8) (ctx->registers.programCounter & 0x00FF);
    busWrite(ctx->registers.stackPointer, pcl);

    ctx->registers.programCounter = ctx->latchedAddr16;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrReturn(void)
{
  CpuContext* ctx =cpuGetContext();

  // - - - M2: Read LSB from stack into Z latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.stackPointer++);
    return false;
  }

  // - - - M3: Read MSB from Stack into W latch 
  if (ctx->mCycle == M3)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.stackPointer++);
    ctx->latchedAddr16 = (u16) ((msb << 8) | lsb);
    return false;
  }

  // - --  M4: Update PC with the return address 
  if (ctx->mCycle == M4)
  {
    ctx->registers.programCounter = ctx->latchedAddr16;
    return true;
  }

  return true;
}

ExecStatus instrReturnConditional(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: condition check 
  if (ctx->mCycle == M2)
  {
    if (!cpuCheckCondition(instr->opcode)) return true;
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Pop PCL (LSB) from stack 
  if (ctx->mCycle == M3)
  {
    ctx->latchedVal8 = busRead(ctx->registers.stackPointer++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - - M4: Pop PCH (MSB) from Stack 
  if (ctx->mCycle == M4)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.stackPointer++);
    ctx->latchedAddr16 = (u16) ((msb << 8) | lsb);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M5: Update PC 
  if (ctx->mCycle == M5)
  {
    ctx->registers.programCounter = ctx->latchedAddr16;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrReturnInterrupt(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Read LSB from stack into Z latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.stackPointer++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Read MSB from stack into latch 
  if (ctx->mCycle == M3)
  {
    u8 lsb = ctx->latchedVal8;
    u8 msb = busRead(ctx->registers.stackPointer++);
    ctx->latchedAddr16 = (u16) ((msb << 8) | lsb);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Update PC and enable interrupt immediately 
  if (ctx->mCycle == M4)
  {
    ctx->registers.programCounter = ctx->latchedAddr16;
    ctx->ime                      = true;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrRestart(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Internal decrement 
  if (ctx->mCycle == M2)
  {
    ctx->registers.stackPointer--;
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Psuh PCH 
  if (ctx->mCycle == M3)
  {
    u8 pch = (u8) ((ctx->registers.programCounter & 0xFF00) >> 8);
    busWrite(ctx->registers.stackPointer, pch);
    ctx->registers.stackPointer--;
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Push PCL and perform the jump 
  if (ctx->mCycle == M4)
  {
    u8 pcl = (u8) (ctx->registers.programCounter & 0x00FF);
    busWrite(ctx->registers.stackPointer, pcl);

    // - - - Jump to fixed address 
    ctx->registers.programCounter = (u16) instr->param;
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}
