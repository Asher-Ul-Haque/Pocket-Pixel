#include "cpu.h"
#include "bus.h"
#include "cartridge.h"
#include "../../GameBoyCore.h"
#include "../../ForgeLibrary/include/asserts.h"
#include "../../ForgeLibrary/include/logger.h"

static CPUContext   cpuCTX  = {0};
static int emuCycles = 0;
#define POSSIBLE_INSTRUCTION_COUNT 256


static Instruction instructions[POSSIBLE_INSTRUCTION_COUNT] =
  {
    // - - - Hex because easy to reference here: https://meganesulli.com/static/851d34afbc4673ee915a8233fda67922/78d47/opcode-tables-screenshot.png
    [0x00] = {INSTRUCTION_NOP,          ADDRESS_MODE_IMP},
    [0x05] = {INSTRUCTION_DECREMENT,    ADDRESS_MODE_R, REG_B},
    [0x0E] = {INSTRUCTION_LOAD,         ADDRESS_MODE_R_D8, REG_C},
    [0xAF] = {INSTRUCTION_XOR,          ADDRESS_MODE_R, REG_A},
    [0xC3] = {INSTRUCTION_JUMP,         ADDRESS_MODE_D16},
  };

FORGE_API void cpuInit()
{ FORGE_LOG_INFO("Started the emulator"); }

FORGE_API void cpuTick()
{
  if (!cpuCTX.halted)
  {
    FORGE_LOG_TRACE("Program counter : %d", cpuCTX.registerFile.programCounter);

    // - - - Fetch instruction
    cpuCTX.currentOpcode     = cartridgeRead(cpuCTX.registerFile.programCounter++);
    cpuCTX.memDest           = 0;
    cpuCTX.destIsMemory      = false;
    Instruction currentInstr = instructions[cpuCTX.currentOpcode];

    // - - - fetch the data
    switch (currentInstr.mode)
    {
      // - - - Implied mode : nothing needs to be read
      case ADDRESS_MODE_IMP   : return; 

      // - - - Register : 
      case ADDRESS_MODE_R     : 
        {
          cpuCTX.readData = cpuReadRegister(currentInstr.reg1);
          return;
        }

      case ADDRESS_MODE_R_D8   :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          return;
        }

      case ADDRESS_MODE_D16    : 
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          cpuCTX.readData = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;
          return;
        }

      default : TODO_COMMENT("For now only one addressing mode : Implied")
    }
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
