#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "instruction.h"

// - - - What the CPU is - - -

typedef struct 
{
  u8  accumulator;
  u8  flags;
  u8  b;
  u8  c;
  u8  d;
  u8  e;
  u8  h;
  u8  l;
  u16 programCounter;
  u16 stackPointer;
} RegisterFile;

typedef struct 
{
  RegisterFile  regs;

  u16           readData;
  u16           memDest;
  bool          destIsMem;
  u8            currentOpcode;
  Instruction*  currentInstruction;

  bool          halted;
  bool          stepping;

  bool          interruptMasterEnabled;
  bool          enablingIme;
  u8            interrupt;
  u8            interruptFlags;
} CPUcontext;


// - - - CPU function - - -

// - - - get register file
FORGE_API RegisterFile* cpuGetRegisters();

// - - - cpu
FORGE_API void cpuInit();
FORGE_API bool cpuStep();

// - - - instruction execution
typedef void (*INSTRUCTION_PROCESSOR)(CPUcontext *);
FORGE_API INSTRUCTION_PROCESSOR InstructionGetProcessor(InstructionType TYPE);

// - - - cpu flags
#define CPU_FLAG_Z BIT(CTX->regs.flags, 7)
#define CPU_FLAG_N BIT(CTX->regs.flags, 6)
#define CPU_FLAG_H BIT(CTX->regs.flags, 5)
#define CPU_FLAG_C BIT(CTX->regs.flags, 4)

// - - - register operations
FORGE_API u16  cpuReadRegister(RegType REG);
FORGE_API void cpuSetRegister(RegType REG, u16 VAL);
FORGE_API u8   cpuReadRegister8(RegType REG);
FORGE_API void cpuSetRegister8(RegType REG, u8 VAL);

// - - - interrupt
FORGE_API u8   cpuGetInterrupt();
FORGE_API void cpuSetInterrupt(u8 INRPT);
FORGE_API u8   cpuGetInterruptFlags();
FORGE_API void cpuSetInterruptFlags(u8 VALUE);

// - - - instructions to string
FORGE_API void instructionToStr(CPUcontext* CTX, char* STR);

#ifdef __cplusplus
}
#endif
