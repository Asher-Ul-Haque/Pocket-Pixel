#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/stack.h"
#include "../../../GameBoyCore.h"


// - - - Helpers - - -


void cpuSetFlags(CPUContext* CTX, i8 Z, i8 N, i8 H, i8 C)
{
  if (Z != -1)
  {
    CTX->registerFile.flags &= ~(1 << 7);
    CTX->registerFile.flags |= (Z & 1) << 7;
  }
  if (N != -1)
  {
    CTX->registerFile.flags &= ~(1 << 6);
    CTX->registerFile.flags |= (N & 1) << 6;
  }
  if (H != -1)
  {
    CTX->registerFile.flags &= ~(1 << 5);
    CTX->registerFile.flags |= (H & 1) << 5;
  }
  if (C != -1)
  {
    CTX->registerFile.flags &= ~(1 << 4);
    CTX->registerFile.flags |= (C & 1) << 4;
  }
}

// - - - check whether to jump or not
static bool checkCondition(CPUContext* CTX)
{
  bool checkCondition = true;

  bool z =  ((CTX->registerFile.flags >> 7) & 1);
  bool c =  ((CTX->registerFile.flags >> 4) & 1);

  switch (CTX->currentInst->cond) 
  {
    case CHECK_NONE     : { checkCondition = true; break; }
    case CHECK_CARRY    : { checkCondition = c;    break; }
    case CHECK_NO_CARRY : { checkCondition = !c;   break; }
    case CHECK_ZERO     : { checkCondition = z;    break; }
    case CHECK_NOT_ZERO : { checkCondition = !z;   break; }
  }

  return checkCondition;
}

// - - - goto
static void gotoAddr(CPUContext* CTX, u16 ADDR, bool PUSH)
{
  if (checkCondition(CTX))
  {
    if (PUSH) 
    {
      cycles(2);
      stackPush16(CTX->registerFile.programCounter);
    }
    CTX->registerFile.programCounter = ADDR;
    cycles(1);
  }
}

static bool is16bit(RegisterType TYPE)
{ return TYPE >= REG_AF; }


// - - - Instruction Processors - - - 

// - - - None : Means not implemented yet
static void procNone(CPUContext* CTX) {}

// - - - Load instruction 
static void procLoad(CPUContext* CTX)
{
  if (CTX->destIsMemory)
  {
    // - - - Check if 16 bit register 
    if (is16bit(CTX->currentInst->reg2))
    {
      cycles(1);
      busWrite16(CTX->memDest, CTX->readData);
    }
    else busWrite(CTX->memDest, CTX->readData);

    return;
  }

  if (CTX->currentInst->mode == ADDR_MODE_HL_SPR)
  {
    u8 halfCarryFlag = (cpuReadRegister(CTX->currentInst->reg2) & 0xF) + 
      (CTX->readData & 0xF) >= 0x10;
    u8 carryFlag     = (cpuReadRegister(CTX->currentInst->reg2) & 0xFF) + 
      (CTX->readData & 0xFF) >= 0x100;

    cpuSetFlags(CTX, 0, 0, halfCarryFlag, carryFlag);
    cpuSetRegister(CTX->currentInst->reg1, cpuReadRegister(CTX->currentInst->reg2) + (i8)(CTX->readData));

    return;
  }

  cpuSetRegister(CTX->currentInst->reg1, CTX->readData);
}

// - - - Load Half 
static void procLoadHalf(CPUContext* CTX)
{
  if (CTX->currentInst->reg1 == REG_A)  cpuSetRegister(REG_A, busRead(0xFF00 | CTX->readData)); 
  else                                  busWrite(0xFF00 | CTX->readData, CTX->registerFile.accumulator);

  cycles(1);
}

// - - - pop 
static void procPop(CPUContext* CTX)
{
  u16 lo = stackPop();
  cycles(1);
  u16 hi = stackPop();
  cycles(1);

  u16 n = (hi << 8) | lo;
  cpuSetRegister(CTX->currentInst->reg1, n);
  if (CTX->currentInst->reg1 == REG_AF) cpuSetRegister(CTX->currentInst->reg1, n & 0xFFF0);
}

// - - - push 
static void procPush(CPUContext* CTX)
{
  u16 hi = (cpuReadRegister(CTX->currentInst->reg1) >> 8) & 0xFF;
  cycles(1);
  stackPush(hi);

  u16 lo = (cpuReadRegister(CTX->currentInst->reg2) & 0xFF);
  cycles(1);
  stackPush(lo);

  cycles(1);
}

// - - - XOR, XOR accumulator with whatever is read 
static void procXOR(CPUContext* CTX)
{ 
  CTX->registerFile.accumulator ^= CTX->readData & 0xFF;
  cpuSetFlags(CTX, CTX->registerFile.accumulator, 0, 0, 0);
}

// - - - disable interruprts
static void procDI(CPUContext* CTX)
{ CTX->interruptMasterEnabled = false; }


// - - - JUMP Insutructions - - -


// - - - jump insturction
static void procJump(CPUContext* CTX)
{ gotoAddr(CTX, CTX->readData, false); }

// - - - function call 
static void procCall(CPUContext* CTX)
{ gotoAddr(CTX, CTX->readData, true); }

// - - - function call with arguments 
static void procRST(CPUContext* CTX)
{ gotoAddr(CTX, CTX->currentInst->param, true); }

// - - - jump relative 
static void procJumpRelative(CPUContext* CTX)
{
  i8  rel  = (i8)(CTX->readData & 0xFF);
  u16 addr = CTX->registerFile.programCounter + rel;
  gotoAddr(CTX, addr, false);
}

// - - - return 
static void procRet(CPUContext* CTX)
{
  if (CTX->currentInst->cond != CHECK_NONE) cycles(1);

  if (checkCondition(CTX))
  {
    u16 lo = stackPop();
    cycles(1);

    u16 hi = stackPop();
    cycles(1);

    u16 n = (hi << 8) | lo;
    CTX->registerFile.programCounter = n;
    cycles(1);
  }
}

// - - - return from interrupt
static void procReturnInterrupt(CPUContext* CTX)
{
  CTX->interruptMasterEnabled = true;
  procRet(CTX);
}

// - - - increment 
static void procIncrement(CPUContext* CTX)
{
  u16 val = cpuReadRegister(CTX->currentInst->reg1) + 1;

  if (is16bit(CTX->currentInst->reg1)) cycles(1);

  if (CTX->currentInst->reg1 == REG_HL && CTX->currentInst->mode == ADDR_MODE_MR)
  {
    val =  busRead(cpuReadRegister(REG_HL)) + 1;
    val &= 0xFF;
    busWrite(cpuReadRegister(REG_HL), val);
  }
  else 
  {
    cpuSetRegister(CTX->currentInst->reg1, val);
    val = cpuReadRegister(CTX->currentInst->reg1);
  }

  if ((CTX->currentOpcode & 0x03) == 0x03) return;

  cpuSetFlags(CTX, (val == 0), 0, (val & 0x0F) == 0, -1);
}

// - - - decrement 
static void procDecrement(CPUContext* CTX)
{
  u16 val = cpuReadRegister(CTX->currentInst->reg1) - 1;

  if (is16bit(CTX->currentInst->reg1)) cycles(1);

  if (CTX->currentInst->reg1 == REG_HL && CTX->currentInst->mode == ADDR_MODE_MR)
  {
    val =  busRead(cpuReadRegister(REG_HL)) - 1;
    busWrite(cpuReadRegister(REG_HL), val);
  }
  else 
  {
    cpuSetRegister(CTX->currentInst->reg1, val);
    val = cpuReadRegister(CTX->currentInst->reg1);
  }

  if ((CTX->currentOpcode & 0x0B) == 0x0B) return;

  cpuSetFlags(CTX, (val == 0), 1, (val & 0x0F) == 0x0F, -1);
}

// - - - add 
static void procAdd(CPUContext* CTX)
{
  u32  val   = cpuReadRegister(CTX->currentInst->reg1) + CTX->readData;
  bool isbig = is16bit(CTX->currentInst->reg1);

  if (isbig) cycles(1);

  if (CTX->currentInst->reg1 == REG_SP)
  {
    val = cpuReadRegister(CTX->currentInst->reg1) + (i8)CTX->readData;
  }

  int z = (val & 0xFF) == 0;
  int h = (cpuReadRegister(CTX->currentInst->reg1) & 0xF) + (CTX->readData & 0xF) >= 0x10;
  int c = (i8)(cpuReadRegister(CTX->currentInst->reg1) & 0xFF) + (i8)(CTX->readData & 0xFF) > 0x100;

  if (isbig && CTX->currentInst->reg1 != REG_SP)
  {
    z     = -1;
    h     = (cpuReadRegister(CTX->currentInst->reg1) & 0xFFF) + (CTX->readData & 0xFFF) > 0x1000;
    u32 n = ((u32)cpuReadRegister(CTX->currentInst->reg1)) + ((u32)CTX->readData);
    c     = n >= 0x10000;
  }

  cpuSetRegister(CTX->currentInst->reg1, val & 0xFFFF);
  cpuSetFlags(CTX, z, 0, h, c);
}

// - - - add with carry 
static void procAddCarry(CPUContext* CTX)
{
  u16 u = CTX->readData;
  u16 a = CTX->registerFile.accumulator;
  u16 c = CTX->registerFile.flags & (1 << 4);

  CTX->registerFile.accumulator = (a + u + c) & 0xFF;
  cpuSetFlags(
    CTX, 
    (CTX->registerFile.accumulator == 0), 
    0, 
    (a & 0xF) + (u &0xF) + c > 0xF, 
    a + u + c > 0xFF);
}

// - - - sub
static void procSub(CPUContext* CTX)
{
  u16 val = cpuReadRegister(CTX->currentInst->reg1) - CTX->readData;
  int z   = (val == 0);
  int h   = ((int)cpuReadRegister(CTX->currentInst->reg1) & 0xF) - ((int)CTX->readData & 0xF) < 0;
  int c   = ((int)cpuReadRegister(CTX->currentInst->reg1)) - ((int)CTX->readData) < 0;

  cpuSetRegister(CTX->currentInst->reg1, val);
  cpuSetFlags(CTX, z, 1, h, c);
}

// - - - sub with carry 
static void procSBC(CPUContext* CTX)
{
  u8 cpuFlagC = CTX->registerFile.flags & (1 << 4);
  u16 val = CTX->readData + cpuFlagC;

  int z   = cpuReadRegister(CTX->currentInst->reg1) - val == 0;
  int h   = ((int)cpuReadRegister(CTX->currentInst->reg1) & 0xF) 
            - ((int)CTX->readData & 0xF)    
            - ((int)cpuFlagC) < 0;
  int c   = ((int)cpuReadRegister(CTX->currentInst->reg1)) 
            - ((int)CTX->readData)
            - ((int)cpuFlagC) < 0;

  cpuSetRegister(CTX->currentInst->reg1, cpuReadRegister(CTX->currentInst->reg1) - val);
  cpuSetFlags(CTX, z, 1, h, c);
}

// - - - API to get processors - - -

static Processor processors[] = 
  {
    [INSTR_NONE] = procNone,
    [INSTR_NOP]  = procNone,
    [INSTR_LOAD] = procLoad,
    [INSTR_JUMP] = procJump,
    [INSTR_DI]   = procDI,
    [INSTR_XOR]  = procXOR,
    [INSTR_LDH]  = procLoadHalf,
    [INSTR_POP]  = procPop,
    [INSTR_PUSH] = procPush,
    [INSTR_JR]   = procJumpRelative,
    [INSTR_CALL] = procCall,
    [INSTR_RET]  = procRet,
    [INSTR_RETI] = procReturnInterrupt,
    [INSTR_RST]  = procRST,
    [INSTR_DEC]  = procDecrement,
    [INSTR_INC]  = procIncrement,
    [INSTR_ADD]  = procAdd,
    [INSTR_SUB]  = procSub,
    [INSTR_SBC]  = procSBC,
  };

Processor getInstrProcessor(InstructionType TYPE)
{ return processors[TYPE]; }
