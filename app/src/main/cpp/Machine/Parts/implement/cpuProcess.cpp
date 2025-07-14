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


// - - - Instruction Processors - - - 

// - - - None : Means not implemented yet
static void procNone(CPUContext* CTX) {}

// - - - Load instruction 
static void procLoad(CPUContext* CTX)
{
  if (CTX->destIsMemory)
  {
    // - - - Check if 16 bit register 
    if (CTX->currentInst->reg2 >= RegisterType::REG_AF)
    {
      cycles(1);
      busWrite16(CTX->memDest, CTX->readData);
    }
    else busWrite(CTX->memDest, CTX->readData);

    return;
  }

  if (CTX->currentInst->mode == ADDRESS_MODE_HL_SPR)
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

// - - - API to get processors - - -
static Processor processors[] = 
  {
    [INSTRUCTION_NONE] = procNone,
    [INSTRUCTION_NOP]  = procNone,
    [INSTRUCTION_LOAD] = procLoad,
    [INSTRUCTION_JUMP] = procJump,
    [INSTRUCTION_DI]   = procDI,
    [INSTRUCTION_XOR]  = procXOR,
    [INSTRUCTION_LDH]  = procLoadHalf,
    [INSTRUCTION_POP]  = procPop,
    [INSTRUCTION_PUSH] = procPush,
    [INSTRUCTION_JR]   = procJumpRelative,
    [INSTRUCTION_CALL] = procCall,
    [INSTRUCTION_RET]  = procRet,
    [INSTRUCTION_RETI] = procReturnInterrupt,
    [INSTRUCTION_RST]  = procRST
  };

Processor getInstrProcessor(InstructionType TYPE)
{ return processors[TYPE]; }
