#include "../../include/cpu.h"
#include "../../include/emu.h"
#include "../../include/bus.h"
#include "../../include/stack.h"
#include "../../include/common.h"


void cpuSetFlags(CPUcontext* CTX, i8 Z, i8 N, i8 H, i8 C) 
{
  if (Z != -1) BIT_SET(CTX->regs.flags, 7, Z);
  if (N != -1) BIT_SET(CTX->regs.flags, 6, N);
  if (H != -1) BIT_SET(CTX->regs.flags, 5, H);
  if (C != -1) BIT_SET(CTX->regs.flags, 4, C);
}

static void procNone(CPUcontext* CTX) 
{
  FORGE_LOG_ERROR("INVALID INSTRUCTION!");
  FORGE_ASSERT(false);
}

static void procNOP(CPUcontext* CTX) {}

RegType regLookup[] = 
  {
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A
  };

RegType decodeRegister(u8 REG) 
{
  if (REG > 0b111) return RT_NONE;
  return regLookup[REG];
}

static void procCB(CPUcontext* CTX) 
{
  u8       op     = CTX->readData;
  RegType reg    = decodeRegister(op & 0b111);
  u8       bit    = (op >> 3) & 0b111;
  u8       bitOP  = (op >> 6) & 0b11;
  u8       regVal = cpuReadRegister8(reg);

  emuCycles(1);

  if (reg == RT_HL) emuCycles(2);

  switch(bitOP) 
  {
    // - - - BIT
    case 1:
      cpuSetFlags(CTX, !(regVal & (1 << bit)), 0, 1, -1);
      return;

    // - - - RST
    case 2:
      regVal &= ~(1 << bit);
      cpuSetRegister8(reg, regVal);
      return;

    // - - - SET
    case 3:
      regVal |= (1 << bit);
      cpuSetRegister8(reg, regVal);
      return;
  }

  bool flagC = CPU_FLAG_C;

  switch(bit) 
  {
    // - - - RLC
    case 0: 
      {
        bool setC = false;
        u8 result = (regVal << 1) & 0xFF;

        if ((regVal & (1 << 7)) != 0) 
        {
          result |= 1;
          setC = true;
        }

        cpuSetRegister8(reg, result);
        cpuSetFlags(CTX, result == 0, false, false, setC);
      } return;

    // - - - RRC
    case 1: 
      {
        u8 old   = regVal;
        regVal >>= 1;
        regVal  |= (old << 7);

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, false, false, old & 1);
      } return;

    // - - - RL
    case 2: 
      {
        u8 old   = regVal;
        regVal <<= 1;
        regVal  |= flagC;

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, false, false, !!(old & 0x80));
      } return;

    // - - - R
    case 3: 
      {
        u8 old   = regVal;
        regVal >>= 1;
        regVal  |= (flagC << 7);

        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, !regVal, false, false, old & 1);
      } return;

    // - - - SLA
    case 4: 
    {
      u8 old   = regVal;
      regVal <<= 1;

      cpuSetRegister8(reg, regVal);
      cpuSetFlags(CTX, !regVal, false, false, !!(old & 0x80));  
    } return;

    // - - - SRA
    case 5: 
      {
        u8 u = (int8_t)regVal >> 1;
        cpuSetRegister8(reg, u);
        cpuSetFlags(CTX, !u, 0, 0, regVal & 1);
      } return;

    // - - - SWAP
    case 6: 
      {
        regVal = ((regVal & 0xF0) >> 4) | ((regVal & 0xF) << 4);
        cpuSetRegister8(reg, regVal);
        cpuSetFlags(CTX, regVal == 0, false, false, false);
      } return;

    // - - - SRL
    case 7: 
      {
        u8 u = regVal >> 1;
        cpuSetRegister8(reg, u);
        cpuSetFlags(CTX, !u, 0, 0, regVal & 1);
      } return;
  }

  FORGE_LOG_FATAL("INVALID CB: %02X", op);
  FORGE_ASSERT(false);
}

static void procRLCA(CPUcontext* CTX) 
{
  u8   u                = CTX->regs.accumulator;
  bool c                = (u >> 7) & 1;
  u                     = (u << 1) | c;
  CTX->regs.accumulator = u;

  cpuSetFlags(CTX, 0, 0, 0, c);
}

static void procRRCA(CPUcontext* CTX) 
{
  u8 b                    = CTX->regs.accumulator & 1;
  CTX->regs.accumulator >>= 1;
  CTX->regs.accumulator  |= (b << 7);

  cpuSetFlags(CTX, 0, 0, 0, b);
}

static void proc_rla(CPUcontext* CTX) 
{
  u8 u  = CTX->regs.accumulator;
  u8 cf = CPU_FLAG_C;
  u8 c  = (u >> 7) & 1;

  CTX->regs.accumulator = (u << 1) | cf;
  cpuSetFlags(CTX, 0, 0, 0, c);
}

static void procStop(CPUcontext* CTX) 
{ 
  FORGE_LOG_WARNING("Restarting the cpu");
  cpuInit();
}

static void procDAA(CPUcontext* CTX) 
{
  u8  u  = 0;
  i32 fc = 0;

  if (CPU_FLAG_H || (!CPU_FLAG_N && (CTX->regs.accumulator & 0xF) > 9)) 
  { u = 6; }

  if (CPU_FLAG_C || (!CPU_FLAG_N && CTX->regs.accumulator > 0x99)) 
  {
    u |= 0x60;
    fc = 1;
  }

  CTX->regs.accumulator += CPU_FLAG_N ? -u : u;

  cpuSetFlags(CTX, CTX->regs.accumulator == 0, -1, 0, fc);
}

static void procCPL(CPUcontext* CTX) 
{
  CTX->regs.accumulator = ~CTX->regs.accumulator;
  cpuSetFlags(CTX, -1, 1, 1, -1);
}

static void procSCF(CPUcontext* CTX) 
{ cpuSetFlags(CTX, -1, 0, 0, 1); }

static void proc_ccf(CPUcontext* CTX) 
{ cpuSetFlags(CTX, -1, 0, 0, CPU_FLAG_C ^ 1); }

static void procHalt(CPUcontext* CTX) 
{  CTX->halted = true;  }

static void procRRA(CPUcontext* CTX) 
{
  u8 carry = CPU_FLAG_C;
  u8 newC  = CTX->regs.accumulator & 1;

  CTX->regs.accumulator >>= 1;
  CTX->regs.accumulator  |= (carry << 7);

  cpuSetFlags(CTX, 0, 0, 0, newC);
}

static void procAND(CPUcontext* CTX) 
{
  CTX->regs.accumulator &= CTX->readData;
  cpuSetFlags(CTX, CTX->regs.accumulator == 0, 0, 1, 0);
}

static void procXOR(CPUcontext* CTX) 
{
  CTX->regs.accumulator ^= CTX->readData & 0xFF;
  cpuSetFlags(CTX, CTX->regs.accumulator == 0, 0, 0, 0);
}

static void procOR(CPUcontext* CTX) 
{
  CTX->regs.accumulator |= CTX->readData & 0xFF;
  cpuSetFlags(CTX, CTX->regs.accumulator == 0, 0, 0, 0);
}

static void procCP(CPUcontext* CTX) 
{
  int n = (int)CTX->regs.accumulator - (int)CTX->readData;

  cpuSetFlags(
    CTX, 
    n == 0, 
    1, 
    ((int)CTX->regs.accumulator & 0x0F) - ((int)CTX->readData & 0x0F) < 0, 
    n < 0);
}

static void procDI(CPUcontext* CTX) 
{ CTX->interruptMasterEnabled = false; }

static void procEI(CPUcontext* CTX) 
{ CTX->enablingIme = true; }

static bool is16Bit(RegType REG) 
{ return REG >= RT_AF; }

static void procLD(CPUcontext* CTX) 
{
  if (CTX->destIsMem) 
  {
    if (is16Bit(CTX->currentInstruction->reg2)) 
    {
      emuCycles(1);
      busWrite16(CTX->memDest, CTX->readData);
    } 
    else 
    { busWrite(CTX->memDest, CTX->readData); }

    emuCycles(1);
    return;
  }

  if (CTX->currentInstruction->mode == AM_HL_SPR) 
  {
    u8 hflag = (cpuReadRegister(CTX->currentInstruction->reg2) & 0xF)  + (CTX->readData & 0xF)  >= 0x10;
    u8 cflag = (cpuReadRegister(CTX->currentInstruction->reg2) & 0xFF) + (CTX->readData & 0xFF) >= 0x100;

    cpuSetFlags(CTX, 0, 0, hflag, cflag);
    cpuSetRegister (CTX->currentInstruction->reg1, 
    cpuReadRegister(CTX->currentInstruction->reg2) + (i8)CTX->readData);

    return;
  }
  cpuSetRegister(CTX->currentInstruction->reg1, CTX->readData);
}

static void procLDH(CPUcontext* CTX) 
{
  if (CTX->currentInstruction->reg1 == RT_A)   cpuSetRegister(CTX->currentInstruction->reg1, busRead(0xFF00 | CTX->readData));
  else                                          busWrite(CTX->memDest, CTX->regs.accumulator);

  emuCycles(1);
}


static bool checkCondition(CPUcontext* CTX) 
{
  bool z = CPU_FLAG_Z;
  bool c = CPU_FLAG_C;

  switch(CTX->currentInstruction->cond) 
  {
    case CT_NONE : return true;
    case CT_C    : return c;
    case CT_NC   : return !c;
    case CT_Z    : return z;
    case CT_NZ   : return !z;
  }

  return false;
}

static void gotoAddress(CPUcontext* CTX, u16 ADDR, bool PUSH_PC) 
{
  if (checkCondition(CTX)) 
  {
    if (PUSH_PC) 
    {
      emuCycles(2);
      stackPush16(CTX->regs.programCounter);
    }

    CTX->regs.programCounter = ADDR;
    emuCycles(1);
  }
}

static void procJP(CPUcontext* CTX) 
{ gotoAddress(CTX, CTX->readData, false); }

static void procJR(CPUcontext* CTX) 
{
  i8  rel  = (char)(CTX->readData & 0xFF);
  u16 addr = CTX->regs.programCounter + rel;

  gotoAddress(CTX, addr, false);
}

static void procCall(CPUcontext* CTX) 
{ gotoAddress(CTX, CTX->readData, true); }

static void procRST(CPUcontext* CTX) 
{ gotoAddress(CTX, CTX->currentInstruction->param, true); }

static void proc_ret(CPUcontext* CTX) 
{
  if (CTX->currentInstruction->cond != CT_NONE) emuCycles(1);

  if (checkCondition(CTX)) 
  {
    u16 lo = stackPop();
    emuCycles(1);
    u16 hi = stackPop();
    emuCycles(1);

    u16 n = (hi << 8) | lo;
    CTX->regs.programCounter = n;

    emuCycles(1);
  }
}

static void procRETI(CPUcontext* CTX) 
{
  CTX->interruptMasterEnabled = true;
  proc_ret(CTX);
}

static void procPop(CPUcontext* CTX) 
{
  u16 lo = stackPop();
  emuCycles(1);
  u16 hi = stackPop();
  emuCycles(1);

  u16 n = (hi << 8) | lo;

  cpuSetRegister(CTX->currentInstruction->reg1, n);

  if (CTX->currentInstruction->reg1 == RT_AF) cpuSetRegister(CTX->currentInstruction->reg1, n & 0xFFF0);
}

static void procPush(CPUcontext* CTX) 
{
  u16 hi = (cpuReadRegister(CTX->currentInstruction->reg1) >> 8) & 0xFF;
  emuCycles(1);
  stackPush(hi);

  u16 lo = cpuReadRegister(CTX->currentInstruction->reg1) & 0xFF;
  emuCycles(1);
  stackPush(lo);

  emuCycles(1);
}

static void procINC(CPUcontext* CTX) 
{
  u16 val = cpuReadRegister(CTX->currentInstruction->reg1) + 1;

  if (is16Bit(CTX->currentInstruction->reg1)) emuCycles(1);

  if (CTX->currentInstruction->reg1 == RT_HL && CTX->currentInstruction->mode == AM_MR) 
  {
    val = busRead(cpuReadRegister(RT_HL)) + 1;
    val &= 0xFF;
    busWrite(cpuReadRegister(RT_HL), val);
  } 
  else 
  {
    cpuSetRegister(CTX->currentInstruction->reg1, val);
    val = cpuReadRegister(CTX->currentInstruction->reg1);
  }

  if ((CTX->currentOpcode & 0x03) == 0x03) return;

  cpuSetFlags(CTX, val == 0, 0, (val & 0x0F) == 0, -1);
}

static void procDEC(CPUcontext* CTX) 
{
  u16 val = cpuReadRegister(CTX->currentInstruction->reg1) - 1;

  if (is16Bit(CTX->currentInstruction->reg1)) emuCycles(1);

  if (CTX->currentInstruction->reg1 == RT_HL && CTX->currentInstruction->mode == AM_MR) 
  {
    val = busRead(cpuReadRegister(RT_HL)) - 1;
    busWrite(cpuReadRegister(RT_HL), val);
  } 
  else 
  {
    cpuSetRegister(CTX->currentInstruction->reg1, val);
    val = cpuReadRegister(CTX->currentInstruction->reg1);
  }

  if ((CTX->currentOpcode & 0x0B) == 0x0B) return;

  cpuSetFlags(CTX, val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

static void procSUB(CPUcontext* CTX) 
{
  u16 val = cpuReadRegister(CTX->currentInstruction->reg1) - CTX->readData;

  i32 z   = (val == 0);
  i32 h   = ((i32)cpuReadRegister(CTX->currentInstruction->reg1) & 0xF) - ((int)CTX->readData & 0xF) < 0;
  i32 c   = ((i32)cpuReadRegister(CTX->currentInstruction->reg1)) - ((int)CTX->readData) < 0;

  cpuSetRegister(CTX->currentInstruction->reg1, val);
  cpuSetFlags(CTX, z, 1, h, c);
}

static void procSBC(CPUcontext* CTX) 
{
  u8  val = CTX->readData + CPU_FLAG_C;
  i32 z   = cpuReadRegister(CTX->currentInstruction->reg1) - val == 0;

  i32 h = ((int)cpuReadRegister(CTX->currentInstruction->reg1) & 0xF) 
    - ((int)CTX->readData & 0xF) - ((int)CPU_FLAG_C) < 0;
  i32 c = ((int)cpuReadRegister(CTX->currentInstruction->reg1)) 
    - ((int)CTX->readData) - ((int)CPU_FLAG_C) < 0;

  cpuSetRegister(CTX->currentInstruction->reg1, cpuReadRegister(CTX->currentInstruction->reg1) - val);
  cpuSetFlags(CTX, z, 1, h, c);
}

static void procADC(CPUcontext* CTX) 
{
  u16 u = CTX->readData;
  u16 a = CTX->regs.accumulator;
  u16 c = CPU_FLAG_C;

  CTX->regs.accumulator = (a + u + c) & 0xFF;

  cpuSetFlags(
    CTX, 
    CTX->regs.accumulator == 0, 
    0, 
    (a & 0xF) + (u & 0xF) + c > 0xF,
    a + u + c > 0xFF);
}

static void procADD(CPUcontext* CTX) 
{
  u32  val     = cpuReadRegister(CTX->currentInstruction->reg1) + CTX->readData;
  bool is16bit = is16Bit(CTX->currentInstruction->reg1);

  if (is16bit) emuCycles(1);

  if (CTX->currentInstruction->reg1 == RT_SP) 
  { val = cpuReadRegister(CTX->currentInstruction->reg1) + (i8)CTX->readData; }

  i32 z = (val & 0xFF) == 0;
  i32 h = (cpuReadRegister(CTX->currentInstruction->reg1) & 0xF) + (CTX->readData & 0xF) >= 0x10;
  i32 c = (int)(cpuReadRegister(CTX->currentInstruction->reg1) & 0xFF) + (int)(CTX->readData & 0xFF) >= 0x100;

  if (is16bit) 
  {
    z     = -1;
    h     = (cpuReadRegister(CTX->currentInstruction->reg1) & 0xFFF) + (CTX->readData & 0xFFF) >= 0x1000;
    u32 n = ((u32)cpuReadRegister(CTX->currentInstruction->reg1)) + ((u32)CTX->readData);
    c     = n >= 0x10000;
  }

  if (CTX->currentInstruction->reg1 == RT_SP) 
  {
    z = 0;
    h = (cpuReadRegister(CTX->currentInstruction->reg1) & 0xF) + (CTX->readData & 0xF) >= 0x10;
    c = (int)(cpuReadRegister(CTX->currentInstruction->reg1) & 0xFF) + (int)(CTX->readData & 0xFF) >= 0x100;
  }

  cpuSetRegister(CTX->currentInstruction->reg1, val & 0xFFFF);
  cpuSetFlags(CTX, z, 0, h, c);
}

static INSTRUCTION_PROCESSOR processors[] = 
  {
    [IN_NONE]   = procNone,
    [IN_NOP]    = procNOP,
    [IN_LD]     = procLD,
    [IN_LDH]    = procLDH,
    [IN_JP]     = procJP,
    [IN_DI]     = procDI,
    [IN_POP]    = procPop,
    [IN_PUSH]   = procPush,
    [IN_JR]     = procJR,
    [IN_CALL]   = procCall,
    [IN_RET]    = proc_ret,
    [IN_RST]    = procRST,
    [IN_DEC]    = procDEC,
    [IN_INC]    = procINC,
    [IN_ADD]    = procADD,
    [IN_ADC]    = procADC,
    [IN_SUB]    = procSUB,
    [IN_SBC]    = procSBC,
    [IN_AND]    = procAND,
    [IN_XOR]    = procXOR,
    [IN_OR]     = procOR,
    [IN_CP]     = procCP,
    [IN_CB]     = procCB,
    [IN_RRCA]   = procRRCA,
    [IN_RLCA]   = procRLCA,
    [IN_RRA]    = procRRA,
    [IN_RLA]    = proc_rla,
    [IN_STOP]   = procStop,
    [IN_HALT]   = procHalt,
    [IN_DAA]    = procDAA,
    [IN_CPL]    = procCPL,
    [IN_SCF]    = procSCF,
    [IN_CCF]    = proc_ccf,
    [IN_EI]     = procEI,
    [IN_RETI]   = procRETI
  };

INSTRUCTION_PROCESSOR InstructionGetProcessor(InstructionType TYPE) 
{ return processors[TYPE]; }
