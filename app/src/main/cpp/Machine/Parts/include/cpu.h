#pragma once
#include "../../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - What the CPU is - - -

typedef struct
{
  u8 accumulator;
  u8 flags;
  u8 b;
  u8 c;
  u8 d;
  u8 e;
  u8 h;
  u8 l;
  u16 stackPointer;
  u16 programCounter;
} RegisterFile;

typedef enum 
{
  REG_NONE,
  REG_A, REG_F,
  REG_B, REG_C,
  REG_D, REG_E,
  REG_H, REG_L,
  REG_AF,
  REG_BC,
  REG_DE,
  REG_HL,
  REG_SP,
  REG_PC,
  REG_COUNT
} RegisterType;

typedef enum 
{
  INSTRUCTION_NONE,
  INSTRUCTION_NOP,
  INSTRUCTION_LOAD,
  INSTRUCTION_INCREMENT,
  INSTRUCTION_DECREMENT,
  INSTRUCTION_RLCA,
  INSTRUCTION_ADD,
  INSTRUCTION_RRCA,
  INSTRUCTION_STOP,
  INSTRUCTION_RLA,
  INSTRUCTION_JR,
  INSTRUCTION_RRA,
  INSTRUCTION_DAA,
  INSTRUCTION_CPL,
  INSTRUCTION_SCF,
  INSTRUCTION_CCF,
  INSTRUCTION_HALT,
  INSTRUCTION_ADC,
  INSTRUCTION_SUB,
  INSTRUCTION_SBC,
  INSTRUCTION_AND,
  INSTRUCTION_XOR,
  INSTRUCTION_OR,
  INSTRUCTION_CP,
  INSTRUCTION_POP,
  INSTRUCTION_JUMP,
  INSTRUCTION_PUSH,
  INSTRUCTION_RET,
  INSTRUCTION_CB,
  INSTRUCTION_CALL,
  INSTRUCTION_RETI,
  INSTRUCTION_LDH,
  INSTRUCTION_JPHL,
  INSTRUCTION_DI,
  INSTRUCTION_EI,
  INSTRUCTION_RST,
  INSTRUCTION_ERR,
  //C - - - B instructions...
  INSTRUCTION_RLC,
  INSTRUCTION_RRC,
  INSTRUCTION_RL,
  INSTRUCTION_RR,
  INSTRUCTION_SLA,
  INSTRUCTION_SRA,
  INSTRUCTION_SWAP,
  INSTRUCTION_SRL,
  INSTRUCTION_BIT,
  INSTRUCTION_RES,
  INSTRUCTION_SET,
  INSTRUCTION_TYPE_COUNT
} InstructionType;

typedef enum 
{
  ADDRESS_MODE_IMP,
  ADDRESS_MODE_R_D16,
  ADDRESS_MODE_R_R,
  ADDRESS_MODE_MR_R,
  ADDRESS_MODE_R,
  ADDRESS_MODE_R_D8,
  ADDRESS_MODE_R_MR,
  ADDRESS_MODE_R_HLI,
  ADDRESS_MODE_R_HLD,
  ADDRESS_MODE_HLI_R,
  ADDRESS_MODE_HLD_R,
  ADDRESS_MODE_R_A8,
  ADDRESS_MODE_A8_R,
  ADDRESS_MODE_HL_SPR,
  ADDRESS_MODE_D16,
  ADDRESS_MODE_D8,
  ADDRESS_MODE_D16_R,
  ADDRESS_MODE_MR_D8,
  ADDRESS_MODE_MR,
  ADDRESS_MODE_A16_R,
  ADDRESS_MODE_R_A16,
  ADDRESSING_MODE_COUNT
} AddressMode;

typedef enum 
{
  CHECK_NONE,
  CHECK_NOT_ZERO,
  CHECK_ZERO,
  CHECK_NO_CARRY,
  CHECK_CARRY
} ConditionType;

typedef struct
{
  InstructionType type;
  AddressMode     mode;
  RegisterType    reg1;
  RegisterType    reg2;
  ConditionType   cond;
  u8              param;
} Instruction;

typedef struct
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
} CPUContext;


// - - - What the CPU does - - -

FORGE_API void cpuInit();
FORGE_API void cpuTick();
FORGE_API u16  cpuReadRegister(RegisterType TYPE);
FORGE_API void cpuSetRegister(RegisterType TYPE, u16 VAL);
FORGE_API void cpuSetInterrupt(u8 INTERRUPT);
FORGE_API u8   cpuGetInterrupt();

FORGE_API RegisterFile* cpuGetRegisters();

typedef void (*Processor)(CPUContext*);
Processor getInstrProcessor(InstructionType TYPE);


#ifdef __cplusplus
}
#endif
