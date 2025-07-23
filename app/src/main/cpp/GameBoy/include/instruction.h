#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


// - - - Addessing Modes
typedef enum 
{
  AM_IMP,
  AM_R_D16,
  AM_R_R,
  AM_MR_R,
  AM_R,
  AM_R_D8,
  AM_R_MR,
  AM_R_HLI,
  AM_R_HLD,
  AM_HLI_R,
  AM_HLD_R,
  AM_R_A8,
  AM_A8_R,
  AM_HL_SPR,
  AM_D16,
  AM_D8,
  AM_D16_R,
  AM_MR_D8,
  AM_MR,
  AM_A16_R,
  AM_R_A16
} AddressMode;

typedef enum 
{
  RT_NONE,
  RT_A,
  RT_F,
  RT_B,
  RT_C,
  RT_D,
  RT_E,
  RT_H,
  RT_L,
  RT_AF,
  RT_BC,
  RT_DE,
  RT_HL,
  RT_SP,
  RT_PC
} RegType;

typedef enum 
{
  IN_NONE,
  IN_NOP,
  IN_LD,
  IN_INC,
  IN_DEC,
  IN_RLCA,
  IN_ADD,
  IN_RRCA,
  IN_STOP,
  IN_RLA,
  IN_JR,
  IN_RRA,
  IN_DAA,
  IN_CPL,
  IN_SCF,
  IN_CCF,
  IN_HALT,
  IN_ADC,
  IN_SUB,
  IN_SBC,
  IN_AND,
  IN_XOR,
  IN_OR,
  IN_CP,
  IN_POP,
  IN_JP,
  IN_PUSH,
  IN_RET,
  IN_CB,
  IN_CALL,
  IN_RETI,
  IN_LDH,
  IN_JPHL,
  IN_DI,
  IN_EI,
  IN_RST,
  IN_ERR,
  IN_RLC, 
  IN_RRC,
  IN_RL, 
  IN_RR,
  IN_SLA, 
  IN_SRA,
  IN_SWAP, 
  IN_SRL,
  IN_BIT, 
  IN_RES, 
  IN_SET
} InstructionType;

typedef enum 
{
  CT_NONE, 
  CT_NZ, 
  CT_Z, 
  CT_NC, 
  CT_C
} ConditionType;

typedef struct 
{
  InstructionType   type;
  AddressMode       mode;
  RegType           reg1;
  RegType           reg2;
  ConditionType     cond;
  u8                param;
} Instruction;

Instruction* instructionGetByOpcode(u8 OPCODE);
const char*  instructionGetName(InstructionType TYPE);


#ifdef __cplusplus
}
#endif
