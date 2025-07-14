#include "cpu.h"
#include "bus.h"
#include "../../GameBoyCore.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "../../ForgeLibrary/include/logger.h"

static CPUContext   cpuCTX  = {0};
static int emuCycles = 0;
#define POSSIBLE_INSTRUCTION_COUNT 0x100


// - - - Instructions - - - 

static Instruction instructions[POSSIBLE_INSTRUCTION_COUNT] =
  {
    // - - - Hex because easy to reference here: https://meganesulli.com/static/851d34afbc4673ee915a8233fda67922/78d47/opcode-tables-screenshot.png

    // - - - 0x0X
    [0x00] = {INSTRUCTION_NOP,        ADDRESS_MODE_IMP},
    [0x01] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D16, REG_BC},
    [0x02] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_BC,   REG_A},

    [0x05] = {INSTRUCTION_DECREMENT,  ADDRESS_MODE_R,     REG_B},
    [0x06] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_B},

    [0x08] = {INSTRUCTION_LOAD,       ADDRESS_MODE_A16_R, REG_NONE, REG_SP},

    [0x0A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_A,    REG_BC},

    [0x0E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_C},

    // - - - 0x1X
    [0x11] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D16, REG_DE},
    [0x12] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_DE,   REG_A},
    [0x15] = {INSTRUCTION_DECREMENT,  ADDRESS_MODE_R,     REG_D},
    [0x16] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_D},
    [0x1A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_A,    REG_DE},
    [0x1E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_E},

    // - - - 0x2X
    [0x21] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D16, REG_HL},
    [0x22] = {INSTRUCTION_LOAD,       ADDRESS_MODE_HLI_R, REG_HL,   REG_A},
    [0x25] = {INSTRUCTION_DECREMENT,  ADDRESS_MODE_R,     REG_H},
    [0x26] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_H},
    [0x2A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_HLI, REG_A,    REG_HL},
    [0x2E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_L},

    // - - - 0x3X
    [0x31] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D16, REG_SP},
    [0x32] = {INSTRUCTION_LOAD,       ADDRESS_MODE_HLD_R, REG_HL,   REG_A},
    [0x35] = {INSTRUCTION_DECREMENT,  ADDRESS_MODE_R,     REG_HL},
    [0x36] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_D8, REG_HL},
    [0x3A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_HLD, REG_A,    REG_HL},
    [0x3E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_D8,  REG_A},

    // - - - 0x4X
    [0x40] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_B},
    [0x41] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_C},
    [0x42] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_D},
    [0x43] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_E},
    [0x44] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_H},
    [0x45] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_L},
    [0x46] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_B,    REG_HL},
    [0x47] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_B,    REG_A},
    [0x48] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_B},
    [0x49] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_C},
    [0x4A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_D},
    [0x4B] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_E},
    [0x4C] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_H},
    [0x4D] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_L},
    [0x4E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_C,    REG_HL},
    [0x4F] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_C,    REG_A},

    // - - - 0x5X
    [0x50] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_B},
    [0x51] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_C},
    [0x52] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_D},
    [0x53] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_E},
    [0x54] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_H},
    [0x55] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_L},
    [0x56] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_D,    REG_HL},
    [0x57] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_D,    REG_A},
    [0x58] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_B},
    [0x59] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_C},
    [0x5A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_D},
    [0x5B] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_E},
    [0x5C] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_H},
    [0x5D] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_L},
    [0x5E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_E,    REG_HL},
    [0x5F] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_E,    REG_A},

    // - - - 0x6X
    [0x60] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_B},
    [0x61] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_C},
    [0x62] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_D},
    [0x63] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_E},
    [0x64] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_H},
    [0x65] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_L},
    [0x66] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_H,    REG_HL},
    [0x67] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_H,    REG_A},
    [0x68] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_B},
    [0x69] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_C},
    [0x6A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_D},
    [0x6B] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_E},
    [0x6C] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_H},
    [0x6D] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_L},
    [0x6E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_L,    REG_HL},
    [0x6F] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_L,    REG_A},

    // - - - 0x7X
    [0x70] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_B},
    [0x71] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_C},
    [0x72] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_D},
    [0x73] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_E},
    [0x74] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_H},
    [0x75] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_L},
    [0x76] = {INSTRUCTION_HALT},
    [0x77] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_HL,   REG_A},
    [0x78] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_B},
    [0x79] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_C},
    [0x7A] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_D},
    [0x7B] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_E},
    [0x7C] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_H},
    [0x7D] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_L},
    [0x7E] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_A,    REG_HL},
    [0x7F] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_R,   REG_A,    REG_A},
    [0xAF] = {INSTRUCTION_XOR,        ADDRESS_MODE_R,     REG_A},

    [0xC3] = {INSTRUCTION_JUMP,       ADDRESS_MODE_D16},

    // - - - 0xEX
    [0xE2] = {INSTRUCTION_LOAD,       ADDRESS_MODE_MR_R,  REG_C,    REG_A},
    [0xEA] = {INSTRUCTION_LOAD,       ADDRESS_MODE_A16_R, REG_NONE, REG_A},

    // - - - 0xFX
    [0xF2] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_MR,  REG_A,    REG_C},
    [0xF3] = {INSTRUCTION_DI},
    [0xFA] = {INSTRUCTION_LOAD,       ADDRESS_MODE_R_A16, REG_A},
  };

std::string inst_lookup[] = 
  {
    "<NONE>",
    "NOP",
    "LD",
    "INC",
    "DEC",
    "RLCA",
    "ADD",
    "RRCA",
    "STOP",
    "RLA",
    "JR",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "XOR",
    "OR",
    "CP",
    "POP",
    "JP",
    "PUSH",
    "RET",
    "CB",
    "CALL",
    "RETI",
    "LDH",
    "JPHL",
    "DI",
    "EI",
    "RST",
    "IN_ERR",
    "IN_RLC",
    "IN_RRC",
    "IN_RL",
    "IN_RR",
    "IN_SLA",
    "IN_SRA",
    "IN_SWAP",
    "IN_SRL",
    "IN_BIT",
    "IN_RES",
    "IN_SET"
  };

const char* getInstrName(InstructionType TYPE) 
{ return inst_lookup[TYPE].c_str(); }


// - - - CPU - - - 

FORGE_API void cpuInit()
{
  FORGE_LOG_INFO("Started the emulator");
  cpuCTX.registerFile.programCounter = 0x100;
  cpuCTX.registerFile.accumulator    = 0x1;
}


FORGE_API void cpuTick()
{
  if (!cpuCTX.halted)
  {
    u16 oldPC = cpuCTX.registerFile.programCounter;

    // - - - Fetch instruction
    cpuCTX.currentOpcode     = busRead(cpuCTX.registerFile.programCounter++);
    cpuCTX.memDest           = 0;
    cpuCTX.destIsMemory      = false;
    cpuCTX.currentInst       = &instructions[cpuCTX.currentOpcode];

    if (cpuCTX.currentInst->type == INSTRUCTION_NONE)
    {
      std::string message = "Cannot execute instruction : " + std::string(getInstrName(cpuCTX.currentInst->type));
      TODO_COMMENT(message.c_str());
    }

    // - - - fetch the data
    switch (cpuCTX.currentInst->mode)
    {
      // - - - Implied mode : nothing needs to be read
      case ADDRESS_MODE_IMP   : break;

      // - - - Register : 
      case ADDRESS_MODE_R     : 
        {
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg1);
          break;
        }

      // - - - From ROM to register 
      case ADDRESS_MODE_R_D8   :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      // - - - 16 bit registers
      case ADDRESS_MODE_D16    : 
      case ADDRESS_MODE_R_D16  : 
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          cpuCTX.readData = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;
          break;
        }

      // - - - from Register(reg2) to memory (reg1)
      case ADDRESS_MODE_MR_R   :
        {
          cpuCTX.readData     = cpuReadRegister(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;

          // - - - special type with C 
          if (cpuCTX.currentInst->reg1 == REG_C) cpuCTX.memDest |= 0xFF00;
          break;
        }

      // - - - from memory(reg2) to register 
      case ADDRESS_MODE_R_MR    :
        {
          u16 addr = cpuReadRegister(cpuCTX.currentInst->reg2);

          if (cpuCTX.currentInst->reg1 == REG_C) addr |= 0xFF00;

          cpuCTX.readData = busRead(addr);
          cycles(1);

          break;
        }

      // - - - increment the HL registers
      case ADDRESS_MODE_R_HLI   :
        {
          cpuCTX.readData = busRead(cpuCTX.currentInst->reg2);
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDRESS_MODE_R_HLD   :
        {
          cpuCTX.readData = busRead(cpuCTX.currentInst->reg2);
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
          break;
        }
      
      // - - - increment the HL registers
      case ADDRESS_MODE_HLI_R   :
        {
          cpuCTX.readData     = busRead(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = busRead(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDRESS_MODE_HLD_R   :
        {
          cpuCTX.readData     = busRead(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = busRead(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
          break;
        }

      case ADDRESS_MODE_R_A8    :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }
      
      case ADDRESS_MODE_A8_R    :
        {
          cpuCTX.memDest      = busRead(cpuCTX.registerFile.programCounter) | 0xFF00;
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      case ADDRESS_MODE_HL_SPR  :
      case ADDRESS_MODE_D8      :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      case ADDRESS_MODE_A16_R   :
      case ADDRESS_MODE_D16_R   : 
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          cpuCTX.memDest = lo | (hi << 8);
          cpuCTX.destIsMemory = true;
          cpuCTX.registerFile.programCounter += 2;
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg2);

          break;
        }

      case ADDRESS_MODE_MR_D8     :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;

          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;

          break;
        }
      
      case ADDRESS_MODE_MR    :
        {
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cpuCTX.readData     = busRead(cpuReadRegister(cpuCTX.currentInst->reg1));
          cycles(1);

          break;
        }

      case ADDRESS_MODE_R_A16 :
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          u16 addr = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;

          cpuCTX.readData = busRead(addr);
          cycles(1);
          break;
        }

      default : TODO_COMMENT("For now only one addressing mode : Implied")
    }

    // - - - execute the instruction
    FORGE_LOG_INFO(
        "0x%04X : \t\t0x%02X : %-10s \t\t(A=0x%02X, B=0x%02X, C=0x%02X)",
        cpuCTX.registerFile.programCounter - 1,
        cpuCTX.currentOpcode,
        getInstrName(cpuCTX.currentInst->type),
        cpuCTX.registerFile.accumulator,
        cpuCTX.registerFile.b,
        cpuCTX.registerFile.c
    );

    Processor proc = getInstrProcessor(cpuCTX.currentInst->type);
    FORGE_ASSERT_MESSAGE(proc, "Cannot have a null processor for an instruction");
    proc(&cpuCTX);    
  }
}

FORGE_INLINE u16 reverse(u16 N)
{ return ((N & 0xFF00) >> 8 | ((N & 0x00FF) << 8)); }

u16 cpuReadRegister(RegisterType TYPE)
{
  switch (TYPE) 
  {
    case REG_A : return cpuCTX.registerFile.accumulator;
    case REG_F : return cpuCTX.registerFile.flags;
    case REG_B : return cpuCTX.registerFile.b;
    case REG_C : return cpuCTX.registerFile.c;
    case REG_D : return cpuCTX.registerFile.d;
    case REG_E : return cpuCTX.registerFile.e;
    case REG_H : return cpuCTX.registerFile.h;
    case REG_L : return cpuCTX.registerFile.l;

    case REG_AF : return reverse(*(u16*)&cpuCTX.registerFile.accumulator);
    case REG_BC : return reverse(*(u16*)&cpuCTX.registerFile.b);
    case REG_DE : return reverse(*(u16*)&cpuCTX.registerFile.d);
    case REG_HL : return reverse(*(u16*)&cpuCTX.registerFile.h);

    case REG_PC : return cpuCTX.registerFile.programCounter;
    case REG_SP : return cpuCTX.registerFile.stackPointer;

    default : return 0;
  }
}

void cpuSetRegister(RegisterType TYPE, u16 VAL)
{
  switch (TYPE) 
  {
    case REG_A : cpuCTX.registerFile.accumulator = VAL & 0xFF; return; 
    case REG_F : cpuCTX.registerFile.flags       = VAL & 0xFF; return; 
    case REG_B : cpuCTX.registerFile.b           = VAL & 0xFF; return; 
    case REG_C : cpuCTX.registerFile.c           = VAL & 0xFF; return; 
    case REG_D : cpuCTX.registerFile.d           = VAL & 0xFF; return; 
    case REG_E : cpuCTX.registerFile.e           = VAL & 0xFF; return; 
    case REG_H : cpuCTX.registerFile.h           = VAL & 0xFF; return; 
    case REG_L : cpuCTX.registerFile.l           = VAL & 0xFF; return; 

    case REG_AF : *(u16*)&cpuCTX.registerFile.accumulator = reverse(VAL); return;
    case REG_BC : *(u16*)&cpuCTX.registerFile.b           = reverse(VAL); return;
    case REG_DE : *(u16*)&cpuCTX.registerFile.d           = reverse(VAL); return;
    case REG_HL : *(u16*)&cpuCTX.registerFile.h           = reverse(VAL); return;

    case REG_PC : cpuCTX.registerFile.programCounter = VAL; return;
    case REG_SP : cpuCTX.registerFile.stackPointer   = VAL; return;

    default : return;
  }
}
