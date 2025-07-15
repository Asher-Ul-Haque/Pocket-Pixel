#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/stack.h"
#include "../../../GameBoyCore.h"
#include <numeric>


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

RegisterType lookup[] =
  {
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_H,
    REG_L,
    REG_HL,
    REG_A
  };

RegisterType decodeReg(u8 REG)
{
  if (REG > 0b111)   return REG_NONE;
  return lookup[REG];
}


// - - - Instruction Processors - - - 

// - - - None : Means not implemented yet
static void procNone(CPUContext* CTX) {}

// - - - CB
static void procCB(CPUContext* CTX)
{
  u8           op     = CTX->readData;
  RegisterType reg    = decodeReg(op & 0b111);
  u8           bit    = (op >> 3) & 0b111;
  u8           bitOP  = (op >> 6 ) & 0b11;
  u8           regVal = cpuReadRegister8(reg);

  cycles(1);
  if (reg == REG_HL) cycles(2);

  switch (bitOP)
  {
    // - - - BIT
    case 1 :
      cpuSetFlags(CTX, !(regVal & (1 << bit)),  0, 1, -1);
      return;

    // - - - RST 
    case 2 : 
      regVal &= ~(1 << bit);
      cpuSetRegister8(reg, regVal);
      return;

    // - - - SET 
    case 3 :
      regVal |= (1 << bit);
      cpuSetRegister8(reg, regVal);
      return;
  }

  bool cFlag = CTX->registerFile.flags & (1 << 4);

  switch (bit)
  {
    // - - - Rorate left to the carry flag
    case 0 :
      {
        bool setC = false;
        u8 result = (regVal << 1) & 0xFF;

        if ((regVal & (1 << 7)) != 0)
        {
          result |= 1;
          setC = true;
        }

        cpuSetRegister8(reg, result);
        cpuSetFlags(CTX, result == 0, 0, 0, setC);

        return;
      }

    // - - - Rotate Right to the carry flag 
    case 1 :
      {
        u8 old = regVal;
        regVal >>= 1;
        regVal |= (old << 7);

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, 0, 0, old & 1);
      }

    // - - - Rotate Left;
    case 2 : 
      {
        u8 old    = regVal;
        regVal  <<= 1;
        regVal   |=  cFlag;

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, 0, 0, !!(old & 0x80));
        return;
      }

    // - - - Rotate Right
    case 3 :
      {
        u8 old   = regVal;
        regVal >>= 1;
        regVal  |= (cFlag << 7); 

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, 0, 0, old & 1);
        return;
      }

    // - - - Shifht left Arithmentci
    case 4 :
      {
        u8 old   = regVal;
        regVal <<= 1;

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, 0, 0, !!(old & 0x80));
        return;
      }

    // - - - Shight Right Arithmetic  
    case 5 :
      {
        u8 u = (i8)regVal >> 1;

        cpuSetRegister8(reg, u);
        cpuSetFlags(CTX, !u, 0, 0, regVal & 1);
        return;
      }

    // - - - SWAP
    case 6 :
      {
        regVal = ((regVal & 0xF0) >> 4) | ((regVal & 0xF) << 4);

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, regVal == 0, 0, 0, 0);
        return;
      }

    // - - - Shift Right Loical
    case 7 :
      {
        u8 u = regVal >> 1;
        
        cpuSetRegister8(reg, u);
        cpuSetFlags(CTX, !u, 0, 0, regVal & 1);
        return;
      }
  }
  FORGE_LOG_ERROR("INVALID CB : %02X", op);
}

// - - - RLCA
static void procRLCA(CPUContext* CTX)
{
  u8   u = CTX->registerFile.accumulator;
  bool c = (u >> 7) & 1;
  u      = (u << 1) | c;
  CTX->registerFile.accumulator = u;

  cpuSetFlags(CTX, 0, 0, 0, c);
}

// - - - RRCA
static void procRRCA(CPUContext* CTX)
{
  u8 b                            = CTX->registerFile.accumulator;
  CTX->registerFile.accumulator >>= 1;
  CTX->registerFile.accumulator  |= (b << 7);

  cpuSetFlags(CTX, 0, 0, 0, b);
}

// - - - RLA
static void procRLA(CPUContext* CTX)
{
  u8 u      = CTX->registerFile.accumulator;
  u8 cFlag  = (CTX->registerFile.accumulator & (1 << 4));
  u8 c      = (u >> 7) & 1;
  CTX->registerFile.accumulator = (u << 1) | cFlag;

  cpuSetFlags(CTX, 0, 0, 0, c);
}

// - - - RRA
static void procRRA(CPUContext* CTX)
{
  u8 carry = (CTX->registerFile.accumulator & (1 << 4));
  u8 newC = CTX->registerFile.accumulator & 1;

  CTX->registerFile.accumulator >>= 1;
  CTX->registerFile.accumulator  |= (carry << 7);

  cpuSetFlags(CTX, 0, 0, 0, newC);
}

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
  cpuSetFlags(CTX, CTX->registerFile.accumulator == 0, 0, 0, 0);
}

// - - - AND, AND accumulator with whatever is read 
static void procAND(CPUContext* CTX)
{ 
  CTX->registerFile.accumulator &= CTX->readData;
  cpuSetFlags(CTX, CTX->registerFile.accumulator == 0, 0, 1, 0);
}

// - - - OR, OR accumulator with whatever is read 
static void procOR(CPUContext* CTX)
{ 
  CTX->registerFile.accumulator |= CTX->readData & 0xFF;
  cpuSetFlags(CTX, CTX->registerFile.accumulator == 0, 0, 0, 0);
}

// - - - CP, CP accumulator with whatever is read 
static void procCP(CPUContext* CTX)
{
  int n = (int) CTX->registerFile.accumulator - (int)CTX->readData;
  CTX->registerFile.accumulator ^= CTX->readData & 0xFF;
  cpuSetFlags(
    CTX, 
    n == 0, 
    1, 
    ((int)CTX->registerFile.accumulator & 0x0F) - ((int)CTX->readData & 0x0F) < 0, 
    n < 0);
}

// - - - disable interruprts
static void procDI(CPUContext* CTX)
{ CTX->interruptMasterEnabled = false; }

// - - - enable interrupts 
static void procEI(CPUContext* CTX)
{  CTX->enableIME = true; }

// - - - stop (or restart in my case)
static void procStop(CPUContext* CTX)
{
  CTX->registerFile           = {0};
  CTX->readData               = 0;
  CTX->currentInst            = {0};
  CTX->currentOpcode          = 0;
  CTX->interruptMasterEnabled = false;
  CTX->interrupt              = 0;
  CTX->destIsMemory           = false;
  CTX->memDest                = 0;
  CTX->steppingMode           = false;
  cpuInit();
}


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

// - - - daa
static void procDAA(CPUContext* CTX)
{
  u8  u     = 0;
  i32 fc    = 0;
  u8  cFlag = (CTX->registerFile.accumulator & (1 << 4));
  u8  hFlag = (CTX->registerFile.accumulator & (1 << 5));
  u8  nFlag = (CTX->registerFile.accumulator & (1 << 6));
  u8  zFlag = (CTX->registerFile.accumulator & (1 << 7));

  if (hFlag || (!nFlag && (CTX->registerFile.accumulator & 0xF) > 9))
  {
    u = 6;
  }

  if (cFlag || (!nFlag && CTX->registerFile.accumulator > 0x99))
  {
    u |= 0x60;
    fc = 1;
  }

  CTX->registerFile.accumulator += nFlag ? -u : u;

  cpuSetFlags(CTX, (CTX->registerFile.accumulator == 0), -1, 0, fc);
}

// - - - cpl 
static void procCPL(CPUContext* CTX)
{
  CTX->registerFile.accumulator = ~CTX->registerFile.accumulator;
  cpuSetFlags(CTX, -1, 1, 1, -1);
}

// - - - fcf 
static void procSCF(CPUContext* CTX)
{ cpuSetFlags(CTX, -1, 0, 0, 1); }

// - - - ccf 
static void procCCF(CPUContext* CTX)
{
  u8 cFlag = CTX->registerFile.accumulator & (1 << 4);
  cpuSetFlags(CTX, -1, 0, 0, cFlag ^ 1); 
}

// - - - halt
static void procHalt(CPUContext* CTX)
{ CTX->halted = true; }


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
    [INSTR_AND]  = procAND,
    [INSTR_OR]   = procOR,
    [INSTR_CP]   = procCP,
    [INSTR_CB]   = procCB,
    [INSTR_RRCA] = procRRCA,
    [INSTR_RLCA] = procRLCA,
    [INSTR_RRA]  = procRRA,
    [INSTR_RLA]  = procRLA,
    [INSTR_STOP] = procStop,
    [INSTR_CCF]  = procCCF,
    [INSTR_HALT] = procHalt,
    [INSTR_DAA]  = procDAA,
    [INSTR_CPL]  = procCPL,
    [INSTR_SCF]  = procSCF,
    [INSTR_EI]   = procEI,
  };

Processor getInstrProcessor(InstructionType TYPE)
{ return processors[TYPE]; }
