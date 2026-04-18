#pragma once

/**
 * @file instructions.h
 * @brief Instruction metadata for the Game Boy SM83 CPU (unprefixed + CB-prefixed).
 *
 * This module provides:
 * - Enums for instruction type, addressing mode, registers, and conditions.
 * - A compact Instruction struct (metadata only; not execution).
 * - Lookup functions for opcode -> Instruction.
*/

#include <common.h>

// - - - Flag behavior encoding

/**
 * @brief Flag behavior per instruction for Z/N/H/C.
 *
 * Encoded as 2 bits per flag:
 * - FB_KEEP    : flag is unaffected / unchanged
 * - FB_SET     : flag is set to 1
 * - FB_RESET   : flag is reset to 0
 * - FB_DEPENDS : flag depends on the operation result (computed by handler)
*/

typedef enum FlagBehavior
{
  FB_KEEP    = 0,
  FB_SET     = 1,
  FB_RESET   = 2,
  FB_DEPENDS = 3
} FlagBehavior;

/**
 * @brief Packed 8-bit flag behavior (2 bits each: Z,N,H,C).
 *
 * Layout (LSB->MSB):
 * - bits 1:0  Z
 * - bits 3:2  N
 * - bits 5:4  H
 * - bits 7:6  C
*/
typedef u8 FlagPack;

// - - - Helpers for building/extracting FlagPack.
#define FLAGPACK_MAKE(ZERO, NEGATIVE, HALF_CARRY, CARRY) \
  ((FlagPack)((((u8)(ZERO) & 3u) << 0) | (((u8)(NEGATIVE) & 3u) << 2) | (((u8)(HALF_CARRY) & 3u) << 4) | (((u8)(CARRY) & 3u) << 6)))

#define FLAGPACK_GET_ZERO(PACK)       ((FlagBehavior)(((PACK) >> 0) & 3u))
#define FLAGPACK_GET_NEGATIVE(PACK)   ((FlagBehavior)(((PACK) >> 2) & 3u))
#define FLAGPACK_GET_HALF_CARRY(PACK) ((FlagBehavior)(((PACK) >> 4) & 3u))
#define FLAGPACK_GET_CARRY(PACK)      ((FlagBehavior)(((PACK) >> 6) & 3u))


// - - - Core enums - - - 

/**
 * @brief Register operand identifiers used by metadata + trace + generic handlers.
 * Keep RT_NONE = 0 so memset-zero yields "no reg".
*/
typedef enum RegType
{
  RT_NONE = 0,

  // - - - 8-bit 
  RT_A,
  RT_B,
  RT_C,
  RT_D,
  RT_E,
  RT_H,
  RT_L,

  // - - - 16-bit pairs / special
  RT_AF,
  RT_BC,
  RT_DE,
  RT_HL,
  RT_SP,
  RT_PC
} RegType;


/// @brief Conditional execution selector.
typedef enum ConditionType
{
  CT_NONE = 0,
  CT_NZ,
  CT_Z,
  CT_NC,
  CT_C
} ConditionType;

/**
 * @brief Addressing modes (operands) used by decode and generic execution.
 *
 * These should map cleanly to your decoder’s immediate-prefetch logic.
 * Add more as needed; keep names stable once used.
*/
typedef enum AddressMode
{
  AM_IMP = 0,    ///< implied / no operands

  // - - - immediates
  AM_D8,         ///< n8 / e8 immediate
  AM_D16,        ///< n16 / a16 immediate

  // - - - register immediates
  AM_R,          ///< single register operand (e.g., INC B)
  AM_R_R,        ///< reg, reg (e.g., LD B,C; ADD A,B uses this sometimes)
  AM_R_D8,       ///< reg, n8 (e.g., LD A,n8)
  AM_R_D16,      ///< reg16, n16 (e.g., LD HL,n16)

  // - - - memory through register pair
  AM_MR_R,       ///< (rr), r (e.g., LD (BC),A ; LD (DE),A ; LD (HL),A)
  AM_R_MR,       ///< r, (rr) (e.g., LD A,(BC) ; LD A,(DE) ; LD A,(HL))
  AM_MR_D8,      ///< (HL), n8

  // - - - absolute memory
  AM_A16_R,      ///< (a16), r
  AM_R_A16,      ///< r, (a16)

  // - - - high RAM / IO
  AM_A8_R,       ///< (0xFF00 + a8), r  (LDH (a8),A)
  AM_R_A8,       ///< r, (0xFF00 + a8)  (LDH A,(a8))
  AM_MR_C,       ///< (0xFF00 + C), A   (LDH (C),A)   (we can encode as mode + regs)
  AM_R_MR_C      ///< A, (0xFF00 + C)   (LDH A,(C))
} AddressMode;

/// @brief Instruction categories (mnemonics).
typedef enum InstructionType
{
  IN_NONE = 0,

  IN_NOP,
  IN_LD,
  IN_LDH,
  IN_INC,
  IN_DEC,

  // - - - rotates (non-CB)
  IN_RLCA,
  IN_RRCA,
  IN_RLA,
  IN_RRA,

  IN_DAA,
  IN_CPL,
  IN_SCF,
  IN_CCF,

  // - - - control
  IN_STOP,
  IN_HALT,
  IN_DI,
  IN_EI,

  // - - - jumps/calls
  IN_JP,
  IN_JPHL,   ///< JP (HL) (special-case mnemonic)
  IN_JR,
  IN_CALL,
  IN_RET,
  IN_RETI,
  IN_RST,

  // - - - stack
  IN_PUSH,
  IN_POP,

  // - - - alu
  IN_ADD,
  IN_ADC,
  IN_SUB,
  IN_SBC,
  IN_AND,
  IN_XOR,
  IN_OR,
  IN_CP,

  // - - - prefix
  IN_CB,

  // - - - CB group names 
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
  IN_SET,

  IN_ERR
} InstructionType;


/**
 * @brief Instruction metadata record for a single opcode.
 *
 * This is *not* the decoded runtime state; it is static metadata used by decode/trace
 * and by generic handlers (e.g., LD r,d8 using reg1 + imm8).
*/
typedef struct Instruction
{
  InstructionType type;     ///< mnemonic/category
  AddressMode     mode;     ///< operand/addressing mode

  RegType         reg1;     ///< primary register operand (dest or first operand)
  RegType         reg2;     ///< secondary register operand (src or second operand)

  ConditionType   cond;     ///< branch condition, if applicable

  u8              bytes;    ///< total instruction length in bytes (1..3, 2 for CB-prefixed included)
  u8              mCycles;  ///< nominal M-cycles
  u8              mCyclesAlt; ///< alternate M-cycles when condition passes/fails (as used by JR/RET/CALL/JP cc)

  FlagPack        flags;    ///< packed Z/N/H/C behavior (2 bits each)

  u8              param;    ///< misc parameter (bit index, rst vector/8, etc.), family-defined
} Instruction;


// - - - Lookup API - - -

/// @brief Lookup unprefixed opcode metadata.
const Instruction* instructionGetByOpcode(u8 OPCODE);

/// @brief Lookup CB-prefixed opcode metadata (the byte after 0xCB).
const Instruction* instructionGetByCBOpcode(u8 CB_OPCODE);

/// @brief Get printable mnemonic name for an InstructionType.
const char* instructionGetName(InstructionType TYPE);

/// @brief Get printable name for a register operand.
const char* instructionGetRegName(RegType REG);
