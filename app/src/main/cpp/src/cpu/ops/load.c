#include <cpu/alu.h>
#include <bus.h>
#include <cpu/cpu.h>
#include <cpu/instruction.h>
#include <cpu/registers.h>
#include <cpu/ops.h>

ExecStatus instrLoadRegReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  u8 val = cpuGetReg8(instr->reg2);
  cpuSetReg8(instr->reg1, val);

  return EXEC_STATUS_DONE_IMMEDIATE;
}

ExecStatus instrLoadReg8bitImm(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u8 val = busRead(ctx->registers.programCounter++);
    cpuSetReg8(instr->reg1, val);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadRegHL(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Address Bus = HL, Data Bus = [HL]
  if (ctx->mCycle == M2)
  {
    u16 addr = cpuGetReg16(RT_HL);
    u8  data = busRead(addr);

    cpuSetReg8(instr->reg1, data);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHLReg(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2) 
  {
    // - - - M2: Address Bus = HL, Data Bus = Register (reg2)
    u16 address = cpuGetReg16(RT_HL);
    u8  val     = cpuGetReg8(instr->reg2);
  
    busWrite(address, val);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHL8bitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Fetch the immediate byte into latch 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_DONE;
  }

  // - - - M3: Write the latched byte to [HL] 
  if (ctx->mCycle == M3)
  {
    u16 addr = cpuGetReg16(RT_HL);
    busWrite(addr, ctx->latchedVal8);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadReg16A(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2) 
  { 
    u16 addr = cpuGetReg16(instr->reg1);
    busWrite(addr, ctx->registers.a);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadAReg16(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  if (ctx->mCycle == M2)
  {
    u16 addr          = cpuGetReg16(instr->reg2);
    ctx->registers.a  = busRead(addr);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoad16BitImmA(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - cycle 2: (M2): Addr = PC, Data = Z <= mem, PC++
  if (ctx->mCycle == M2)
  {
    ctx->latchedAddr16 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - cycle 3 (M3): Addr = PC, Data = W <= mem, PC++
  if (ctx->mCycle == M3)
  {
    u16 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 |= (msb << 8);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - cycle 4 (M4): Addr = WZ, Data = mem <- A 
  if (ctx->mCycle == M4)
  {
    busWrite(ctx->latchedAddr16, ctx->registers.a);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadA16BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  if (ctx->mCycle == M2)
  {
    ctx->latchedAddr16 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  if (ctx->mCycle == M3)
  {
    u16 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 |= (msb << 8);
    return EXEC_STATUS_CONTINUE;
  }

  if (ctx->mCycle == M4)
  {
    ctx->registers.a = busRead(ctx->latchedAddr16);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHighAC(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Read from [0xFF00 + C] into A
  if (ctx->mCycle == M2)
  {
    u16 addr          = 0xFF00 | ctx->registers.c;
    ctx->registers.a  = busRead(addr);
    return EXEC_STATUS_DONE;
  }
  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHighCA(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Write A into [0xFF00 + C]
  if (ctx->mCycle == M2)
  {
    u16 addr = 0xFF00 | ctx->registers.c;
    busWrite(addr, ctx->registers.a);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHighA8BitImm(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Fetch the offset byte
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Address Bus = 0xFF00 + Z, Data Bus = A <= mem 
  if (ctx->mCycle == M3)
  {
    u16 addr = 0xFF00 | ctx->latchedVal8;
    ctx->registers.a = busRead(addr);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHigh8BitImmA(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2:Fetch the offset byte 
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Address Bus = 0xFF00 + Z, Data bus = A <= mem 
  if (ctx->mCycle == M3)
  {
    u16 addr = 0xFF00 | ctx->latchedVal8;
    ctx->registers.a = busRead(addr);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadAHLIncDec(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Read from [HL]
  if (ctx->mCycle == M2)
  {
    u16 addr          = cpuGetReg16(RT_HL);
    ctx->registers.a  = busRead(addr);
  
    if (instr->opcode == OP_LOAD_A_HL_INCR) cpuSetReg16(RT_HL, addr + 1);
    else                                    cpuSetReg16(RT_HL, addr - 1);

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHLIncDecA(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Bus access 
  if (ctx->mCycle == M2)
  {
    u16 addr = cpuGetReg16(RT_HL);
    busWrite(addr, ctx->registers.a);

    // - - - M2: IDU Action
    if (instr->opcode == OP_LOAD_HL_INCR_A) cpuSetReg16(RT_HL, addr + 1);
    else                                    cpuSetReg16(RT_HL, addr - 1);

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoad16BitReg16BitImm(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Read LSB (Z)
  if (ctx->mCycle == M2)
  {
    ctx->latchedAddr16 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Read MSB (W) and commit 
  if (ctx->mCycle == M3)
  {
    u16 msb       = busRead(ctx->registers.programCounter++);
    u16 fullValue = (msb << 8) | ctx->latchedAddr16;
    cpuSetReg16(instr->reg1, fullValue);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoad16BitImmSP(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Read LSB of address 
  if (ctx->mCycle == M2)
  {
    ctx->latchedAddr16 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - --  M3: Read MSB of address 
  if (ctx->mCycle == M3)
  {
    u16 msb = busRead(ctx->registers.programCounter++);
    ctx->latchedAddr16 |= (msb << 8);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M4: Write SP Low byte to [nn], then nn++
  if (ctx->mCycle == M4)
  {
    busWrite(ctx->latchedAddr16, (u8) (ctx->registers.stackPointer & 0x0FF));
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M5: Write SP High byte to [nn + 1]
  if (ctx->mCycle == M5)
  {
    busWrite(ctx->latchedAddr16 + 1, (u8) (ctx->registers.stackPointer >> 8));
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadSpHl(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Perform the 16-bit transfer 
  if (ctx->mCycle == M2)
  {
    ctx->registers.stackPointer = cpuGetReg16(RT_HL);
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrPop(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;

  // - - - M2: Read LSB (Z), then SP <= SP + 1
  if (ctx->mCycle == M2) 
  {
    ctx->latchedVal8 = busRead(ctx->registers.stackPointer++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Read MSB (W), then SP <= SP + 1 
  if (ctx->mCycle == M3)
  {
    u8  msb     = busRead(ctx->registers.stackPointer++);
    u16 fullVal = (msb << 8) | ctx->latchedVal8;

    if (instr->reg1 == RT_AF) cpuSetReg16(RT_AF, fullVal & 0xFFF0);
    else                      cpuSetReg16(instr->reg1, fullVal);

    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrPush(void)
{
  CpuContext*         ctx   = cpuGetContext();
  const Instruction*  instr = ctx->currentInstruction;
  u16                 val   = cpuGetReg16(instr->reg1);

  if (ctx->mCycle == M2)
  {
    ctx->registers.stackPointer--;
    return EXEC_STATUS_CONTINUE;
  }

  if (ctx->mCycle == M3)
  {
    busWrite(ctx->registers.stackPointer, (u8)(val >> 8));
    ctx->registers.stackPointer--;
    return EXEC_STATUS_CONTINUE;
  }

  if (ctx->mCycle == M4)
  {
    busWrite(ctx->registers.stackPointer, (u8)(val & 0xFF));
    return EXEC_STATUS_DONE;
  }

  return EXEC_STATUS_CONTINUE;
}

ExecStatus instrLoadHlSpE8(void)
{
  CpuContext* ctx = cpuGetContext();

  // - - - M2: Fetch signed immediate 'e' into latch Z
  if (ctx->mCycle == M2)
  {
    ctx->latchedVal8 = busRead(ctx->registers.programCounter++);
    return EXEC_STATUS_CONTINUE;
  }

  // - - - M3: Perform the calculation 
  if (ctx->mCycle == M3)
  {
    i8  offset = (i8) ctx->latchedVal8;
    u16 sp     = ctx->registers.stackPointer;

    AluResult16 res = aluAdd16Sp(sp, offset);
    cpuSetReg16(RT_HL, res.result);

    // - - - Apply flags, Z = 0, N = 0, H and C from ALU 
    ctx->registers.f = 0;
    if (res.halfCarry) ctx->registers.f |= FLAG_H;
    if (res.carry)     ctx->registers.f |= FLAG_C;

    return EXEC_STATUS_DONE;
  }
  
  return EXEC_STATUS_CONTINUE;
}
