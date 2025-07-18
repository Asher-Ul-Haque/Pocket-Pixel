#pragma once
#include "../../../defines.h"
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
  u16 stackPointer;
  u16 programCounter;
} RegisterFile;

typedef struct CPUContext
{
  RegisterFile    registerFile;
  u16             readData;
  u16             memDest;
  u8              currentOpcode;
  bool            destIsMemory;
  bool            halted;
  bool            steppingMode;
  bool            interruptMasterEnabled;
  u8              interrupt;
  Instruction*    currentInst;
  bool            enableIME;
  u8              interruptFlags;
} CPUContext;



// - - - What the CPU does - - -

FORGE_API void cpuInit();
FORGE_API void cpuTick();
FORGE_API u16  cpuReadRegister(RegisterType TYPE);
FORGE_API void cpuSetRegister(RegisterType TYPE, u16 VAL);
FORGE_API u8   cpuReadRegister8(RegisterType TYPE);
FORGE_API void cpuSetRegister8(RegisterType TYPE, u8 VAL);
FORGE_API void cpuSetInterrupt(u8 INTERRUPT);
FORGE_API u8   cpuGetInterrupt();
FORGE_API u8   cpuGetInterruptFlags();
FORGE_API void cpuSetInterruptFlags(u8 VALUE);

FORGE_API RegisterFile* cpuGetRegisters();

typedef void (*Processor)(CPUContext*);
Processor getInstrProcessor(InstructionType TYPE);


#ifdef __cplusplus
}
#endif
