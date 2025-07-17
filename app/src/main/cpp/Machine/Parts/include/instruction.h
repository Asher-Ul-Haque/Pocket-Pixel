#pragma once
#include "../../../defines.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
  ADDR_MODE_IMP,
  ADDR_MODE_R_D16,
  ADDR_MODE_R_R,
  ADDR_MODE_MR_R,
  ADDR_MODE_R,
  ADDR_MODE_R_D8,
  ADDR_MODE_R_MR,
  ADDR_MODE_R_HLI,
  ADDR_MODE_R_HLD,
  ADDR_MODE_HLI_R,
  ADDR_MODE_HLD_R,
  ADDR_MODE_R_A8,
  ADDR_MODE_A8_R,
  ADDR_MODE_HL_SPR,
  ADDR_MODE_D16,
  ADDR_MODE_D8,
  ADDR_MODE_D16_R,
  ADDR_MODE_MR_D8,
  ADDR_MODE_MR,
  ADDR_MODE_A16_R,
  ADDR_MODE_R_A16,
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

typedef enum 
{
  INSTR_NONE,
  INSTR_NOP,
  INSTR_LOAD,
  INSTR_INC,
  INSTR_DEC,
  INSTR_RLCA,
  INSTR_ADD,
  INSTR_RRCA,
  INSTR_STOP,
  INSTR_RLA,
  INSTR_JR,
  INSTR_RRA,
  INSTR_DAA,
  INSTR_CPL,
  INSTR_SCF,
  INSTR_CCF,
  INSTR_HALT,
  INSTR_ADC,
  INSTR_SUB,
  INSTR_SBC,
  INSTR_AND,
  INSTR_XOR,
  INSTR_OR,
  INSTR_CP,
  INSTR_POP,
  INSTR_JUMP,
  INSTR_PUSH,
  INSTR_RET,
  INSTR_CB,
  INSTR_CALL,
  INSTR_RETI,
  INSTR_LDH,
  INSTR_JPHL,
  INSTR_DI,
  INSTR_EI,
  INSTR_RST,
  INSTR_ERR,
  //C - - - B instructions...
  INSTR_RLC,
  INSTR_RRC,
  INSTR_RL,
  INSTR_RR,
  INSTR_SLA,
  INSTR_SRA,
  INSTR_SWAP,
  INSTR_SRL,
  INSTR_BIT,
  INSTR_RES,
  INSTR_SET,
  INSTRUCTION_TYPE_COUNT
} InstructionType;

typedef enum 
{
  REG_NONE,
  REG_A, 
  REG_F,
  REG_B, 
  REG_C,
  REG_D, 
  REG_E,
  REG_H, 
  REG_L,
  REG_AF,
  REG_BC,
  REG_DE,
  REG_HL,
  REG_SP,
  REG_PC,
  REG_COUNT
} RegisterType;

typedef struct
{
  InstructionType type;
  AddressMode     mode;
  RegisterType    reg1;
  RegisterType    reg2;
  ConditionType   cond;
  u8              param;
} Instruction;


struct CPUContext;

FORGE_API Instruction*  getInstrByOpcode(u8 OPCODE);
FORGE_API void          getInstrStr(Instruction* INSTR, u8 READ_VALUE, u16 PROGRAM_COUNTER, char* STR);

#ifdef __cplusplus
}
#endif
