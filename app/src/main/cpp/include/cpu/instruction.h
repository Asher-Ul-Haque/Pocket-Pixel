/**
 * @file instructions.h
 * @brief Instruction metadata for the Game Boy SM83 CPU (unprefixed + CB-prefixed).
 *
 * This module provides:
 * - Enums for instruction type, addressing mode, registers, and conditions.
 * - A compact Instruction struct (metadata only; not execution).
 * - Lookup functions for opcode -> Instruction.
*/

#pragma once
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
  CT_NONE,
  CT_NOT_ZERO,
  CT_ZERO,
  CT_NOT_CARRY,
  CT_CARRY,

  CT_COUNT
} ConditionType;

/// @brief Enum to represent an instruction's state 
typedef enum ExecStatus
{
  EXEC_STATUS_CONTINUE,       ///< Instruction is not done and has consumed one m cycle
  EXEC_STATUS_DONE,           ///< Instruction done and consumed an M cycle 
  EXEC_STATUS_DONE_IMMEDIATE, ///< Instruction finished using 0 extra cycles
} ExecStatus;

/**
 * @enum AddressMode
 * @brief Represents the bus-behavioral category of an instruction.
 * Each mode dictates a specific schedule of M-cycles and bus interactions.
 */
typedef enum AddressMode
{
  AM_IMPLIED,    ///< implied / no operands (e.g. NOP, DI)

  // - - - register immediates
  AM_REG,            ///< single register operand (e.g., INC B)
  AM_REG_REG,        ///< reg, reg (e.g., LD B,C; ADD A,B uses this sometimes)
  AM_REG_8_BIT_IMM,  ///< reg, n8 (e.g., LD A,n8)
  AM_REG_16_BIT_IMM, ///< reg16, n16 (e.g., LD HL,n16)

  // - - - Memory register pairs
  AM_REG_MEM,  ///< register + memory (e.g. LD A, (BC))
  AM_MEM_REG,  ///< memory + register (e.g. LD (BC), A)

  // - - - Register immediate address pair 
  AM_REG_16_BIT_ADDR, ///< reg, 16 bit addr (e.g., LD A, (a16))
  AM_16_BIT_ADDR_REG, ///< 16 bit add, register (e.g., LD(a16), A)

  // - - - Increment and Decrement 
  AM_HL_INCR_REG, ///< HL increment + Register (e.g., LD (HL+), A)
  AM_REG_HL_INCR, ///< Register + HL increment (e.g., LD A, (HL+))
  AM_HL_DECR_REG, ///< HL Decrement + Register (e.g. LD (HL-), A)
  AM_REG_HL_DECR, ///< Register + HL Drecrement (e.g. LD, A, (HL-))

  // - - - high RAM / IO
  AM_HIGH_RAM_ADDR,     ///< High RAM (LDH): (e.g. LDH, A, (a8))
  AM_ADDR_HIGH_RAM,     ///< High Ram (LDH) (.e.g LDH, (a8), A)
  AM_HIGH_RAM_COMP_REG, ///< High RAM (C): (e.g. LDH, A, (c))
  AM_COMP_REG_HIGH_RAM, ///< High RAM (C): (e.g. LDH, (C), A)

  // - - - Offset
  AM_STACK_PTR_SIGNED_OFFSET, ///< Sp + Signed offset (e.g. LD, HL, SP + e8)

  // - - - Imemediate 
  AM_8_BIT_IMM,  ///< 8 bit immediate (e.g. JP, Z, e8) 
  AM_16_BIT_IMM, ///< 16 bit immedate (e.h. JP, nn)

  AM_COUNT
} AddressMode;

#define CB_OFFSET 0x100
/// @brief Instruction categories (mnemonics).
typedef enum Opcode
{
  // - - - 8-bit loads (Register to Register)
  OP_LOAD_B_B = 0x40, 
  OP_LOAD_B_C = 0x41,
  OP_LOAD_B_D = 0x42,
  OP_LOAD_B_E = 0x43,
  OP_LOAD_B_H = 0x44,
  OP_LOAD_B_L = 0x45,
  OP_LOAD_B_A = 0x47,
  OP_LOAD_C_B = 0x48,
  OP_LOAD_C_C = 0x49,
  OP_LOAD_C_D = 0x4A,
  OP_LOAD_C_E = 0x4B, 
  OP_LOAD_C_H = 0x4C,
  OP_LOAD_C_L = 0x4D,
  OP_LOAD_C_A = 0x4F,
  OP_LOAD_D_B = 0x50,
  OP_LOAD_D_C = 0x51,
  OP_LOAD_D_D = 0x52,
  OP_LOAD_D_E = 0x53,
  OP_LOAD_D_H = 0x54,
  OP_LOAD_D_L = 0x55,
  OP_LOAD_D_A = 0x57,
  OP_LOAD_E_B = 0x58,
  OP_LOAD_E_C = 0x59,
  OP_LOAD_E_D = 0x5A,
  OP_LOAD_E_E = 0x5B,
  OP_LOAD_E_H = 0x5C,
  OP_LOAD_E_L = 0x5D,
  OP_LOAD_E_A = 0x5F,
  OP_LOAD_H_B = 0x60,
  OP_LOAD_H_C = 0x61,
  OP_LOAD_H_D = 0x62,
  OP_LOAD_H_E = 0x63,
  OP_LOAD_H_H = 0x64,
  OP_LOAD_H_L = 0x65,
  OP_LOAD_H_A = 0x67,
  OP_LOAD_L_B = 0x68,
  OP_LOAD_L_C = 0x69,
  OP_LOAD_L_D = 0x6A,
  OP_LOAD_L_E = 0x6B,
  OP_LOAD_L_H = 0x6C,
  OP_LOAD_L_L = 0x6D,
  OP_LOAD_L_A = 0x6F,
  OP_LOAD_A_B = 0x78,
  OP_LOAD_A_C = 0x79,
  OP_LOAD_A_D = 0x7A,
  OP_LOAD_A_E = 0x7B,
  OP_LOAD_A_H = 0x7C,
  OP_LOAD_A_L = 0x7D,
  OP_LOAD_A_A = 0x7F,

  // - - - 8 bit immediate loads 
  OP_LOAD_B_8_BIT_IMM = 0x06,
  OP_LOAD_C_8_BIT_IMM = 0x0E,
  OP_LOAD_D_8_BIT_IMM = 0x16,
  OP_LOAD_E_8_BIT_IMM = 0x1E,
  OP_LOAD_H_8_BIT_IMM = 0x26,
  OP_LOAD_L_8_BIT_IMM = 0x2E,
  OP_LOAD_A_8_BIT_IMM = 0x3E,

  // - - - 8 bit indirect loads from HL 
  OP_LOAD_B_HL = 0x46,
  OP_LOAD_C_HL = 0x4E,
  OP_LOAD_D_HL = 0x56,
  OP_LOAD_E_HL = 0x5E,
  OP_LOAD_H_HL = 0x66,
  OP_LOAD_L_HL = 0x6E,
  OP_LOAD_A_HL = 0x7E,

  // - - - 8 bit indirect stores to HL 
  OP_LOAD_HL_B = 0x70,
  OP_LOAD_HL_C = 0x71,
  OP_LOAD_HL_D = 0x72,
  OP_LOAD_HL_E = 0x73,
  OP_LOAD_HL_H = 0x74,
  OP_LOAD_HL_L = 0x75,
  OP_LOAD_HL_A = 0x77,

  // - - - store to HL using immediate 
  OP_LOAD_HL_8_BIT_IMM = 0x36,

  // - - - store to and from A 
  OP_LOAD_A_BC = 0x0A,
  OP_LOAD_BC_A = 0x02,
  OP_LOAD_DE_A = 0x12,
  OP_LOAD_A_DE = 0x1A,

  // - - - immediate load to A 
  OP_LOAD_A_16_BIT_IMM = 0xFA,
  OP_LOAD_16_BIT_IMM_A = 0xEA,

  // - - - C offset High Ram Loads 
  OP_LOAD_HIGH_C_A = 0xE2,
  OP_LOAD_HIGH_A_C = 0xF2,

  // - - - Load High Ram, between A and immediate
  OP_LOAD_HIGH_8_BIT_IMM_A = 0xE0,
  OP_LOAD_HIGH_A_8_BIT_IMM = 0xF0,

  // - - - Increment Decrement Loads 
  OP_LOAD_HL_INCR_A = 0x22,
  OP_LOAD_A_HL_INCR = 0x2A,
  OP_LOAD_HL_DECR_A = 0x32,
  OP_LOAD_A_HL_DECR = 0x3A,

  // - - - 16 bit register immediate loads 
  OP_LOAD_BC_16_BIT_IMM = 0x01,
  OP_LOAD_DE_16_BIT_IMM = 0x11,
  OP_LOAD_HL_16_BIT_IMM = 0x21,
  OP_LOAD_SP_16_BIT_IMM = 0x31,

  // - - - Load imm, SP 
  OP_LOAD_16_BIT_IMM_SP = 0x08,

  // - - - Load SP, HL 
  OP_LOAD_SP_HL = 0xF9,

  // - - - Push and Pop 
  OP_PUSH_BC = 0xC5,
  OP_PUSH_DE = 0xD5,
  OP_PUSH_HL = 0xE5,
  OP_PUSH_AF = 0xF5,
  OP_POP_BC  = 0xC1,
  OP_POP_DE  = 0xD1,
  OP_POP_HL  = 0xE1,
  OP_POP_AF  = 0xF1,

  // - - - load HL, SP, E8 
  OP_LOAD_HL_SP_E8 = 0xF8,

  // - - - Add A, reg 
  OP_ADD_A_B = 0x80,
  OP_ADD_A_C = 0x81,
  OP_ADD_A_D = 0x82,
  OP_ADD_A_E = 0x83,
  OP_ADD_A_H = 0x84,
  OP_ADD_A_L = 0x85,
  OP_ADD_A_A = 0x87,

  // - - - Indirect A adds 
  OP_ADD_A_HL         = 0x86,
  OP_ADD_A_8_BIT_IMM  = 0xC6,

  // - - - 8 bit ADC reg 
  OP_ADC_A_B = 0x88,
  OP_ADC_A_C = 0x89,
  OP_ADC_A_D = 0x8A,
  OP_ADC_A_E = 0x8B,
  OP_ADC_A_H = 0x8C,
  OP_ADC_A_L = 0x8D,
  OP_ADC_A_A = 0x8F,

  // - - - Indirect 8 bit adds 
  OP_ADC_A_HL         = 0x8E,
  OP_ADC_A_8_BIT_IMM  = 0xCE,

  // - - - 8 bit sub 
  OP_SUB_A_B = 0x90,
  OP_SUB_A_C = 0x91,
  OP_SUB_A_D = 0x92,
  OP_SUB_A_E = 0x93,
  OP_SUB_A_H = 0x94,
  OP_SUB_A_L = 0x95,
  OP_SUB_A_A = 0x97,

  // - - - Indirect Subs 
  OP_SUB_A_HL         = 0x96,
  OP_SUB_A_8_BIT_IMM  = 0xD6,

  // - - - 8 bit SBC A,r 
  OP_SBC_A_B = 0x98,
  OP_SBC_A_C = 0x99,
  OP_SBC_A_D = 0x9A,
  OP_SBC_A_E = 0x9B,
  OP_SBC_A_H = 0x9C,
  OP_SBC_A_L = 0x9D,
  OP_SBC_A_A = 0x9F,

  // - - - Indirect subs 
  OP_SBC_A_HL         = 0x9E,
  OP_SBC_A_8_BIT_IMM  = 0xDE,

  // - - - 8 bit reg compares 
  OP_COMP_A_B = 0xB8,
  OP_COMP_A_C = 0xB9,
  OP_COMP_A_D = 0xBA,
  OP_COMP_A_E = 0xBB,
  OP_COMP_A_H = 0xBC,
  OP_COMP_A_L = 0xBD,
  OP_COMP_A_A = 0xBF,

  // - - - indirect compares 
  OP_COMP_A_HL        = 0xBE,
  OP_COMP_A_8_BIT_IMM = 0xFE,

  // - - - 8 bit Increment
  OP_INC_B = 0x04,
  OP_INC_C = 0x0C,
  OP_INC_D = 0x14,
  OP_INC_E = 0x1C,
  OP_INC_H = 0x24,
  OP_INC_L = 0x2C,
  OP_INC_A = 0x3C,

  // - - - 8 bit decrement 
  OP_DEC_B = 0x05,
  OP_DEC_C = 0x0D,
  OP_DEC_D = 0x15,
  OP_DEC_E = 0x1D,
  OP_DEC_H = 0x25,
  OP_DEC_L = 0x2D,
  OP_DEC_A = 0x3D,

  // - - - Indirect Increment, Decrement
  OP_DEC_HL = 0x35,
  OP_INC_HL = 0x34, 

  // - - - 8 bit AND r 
  OP_AND_A_B = 0xA0,
  OP_AND_A_C = 0xA1,
  OP_AND_A_D = 0xA2,
  OP_AND_A_E = 0xA3,
  OP_AND_A_H = 0xA4,
  OP_AND_A_L = 0xA5,
  OP_AND_A_A = 0xA7,

  // - - - indirect and 
  OP_AND_A_HL         = 0xA6,
  OP_AND_A_8_BIT_IMM  = 0xE6,

  // - - - 8 bit OR r 
  OP_OR_A_B = 0xB0, 
  OP_OR_A_C = 0xB1,
  OP_OR_A_D = 0xB2,
  OP_OR_A_E = 0xB3,
  OP_OR_A_H = 0xB4,
  OP_OR_A_L = 0xB5,
  OP_OR_A_A = 0xB7,

  OP_OR_A_HL        = 0xB6,
  OP_OR_A_8_BIT_IMM = 0xF6,

  // - - - 8 bit XOR 
  OP_XOR_A_B = 0xA8,
  OP_XOR_A_C = 0xA9,
  OP_XOR_A_D = 0xAA,
  OP_XOR_A_E = 0xAB, 
  OP_XOR_A_H = 0xAC,
  OP_XOR_A_L = 0xAD,
  OP_XOR_A_A = 0xAF,

  // - - - Indirect XOR 
  OP_XOR_A_HL         = 0xAE,
  OP_XOR_A_8_BIT_IMM  = 0xEE,

  // - - - Extra arithmetic
  OP_CCF = 0x3F,
  OP_SCF = 0x37,
  OP_DAA = 0x27,
  OP_CPL = 0x2F,

  // - - - 16 bit increment 
  OP_INC_BC     = 0x03,
  OP_INC_DE     = 0x13,
  OP_INC_HL_REG = 0x23,
  OP_INC_SP     = 0x33,

  // - - - 16 bit decrement 
  OP_DEC_BC     = 0x0B,
  OP_DEC_DE     = 0x1B,
  OP_DEC_HL_REG = 0x2B,
  OP_DEC_SP     = 0x3B,

  // - - - 16 bit ADD HL, rr 
  OP_ADD_HL_BC = 0x09,
  OP_ADD_HL_DE = 0x19,
  OP_ADD_HL_HL = 0x29,
  OP_ADD_HL_SP = 0x39,
  OP_ADD_SP_E8 = 0xE8, 

  // - - - Rotates 
  OP_ROTATE_LEFT_CIRCULAR_A   = 0x07,
  OP_ROTATE_RIGHT_CIRCULAR_A  = 0x0F,
  OP_ROTATE_LEFT_A            = 0x17,
  OP_ROTATE_RIGHT_A           = 0x1F,

  OP_CB_PREFIX = 0xCB,

  // - - - Cb rotates left 
  OP_CB_ROTATE_LEFT_CIRCULAR_B = 0x100,
  OP_CB_ROTATE_LEFT_CIRCULAR_C = 0x101,
  OP_CB_ROTATE_LEFT_CIRCULAR_D = 0x102,
  OP_CB_ROTATE_LEFT_CIRCULAR_E = 0x103,
  OP_CB_ROTATE_LEFT_CIRCULAR_H = 0x104,
  OP_CB_ROTATE_LEFT_CIRCULAR_L = 0x105,
  OP_CB_ROTATE_LEFT_CIRCULAR_A = 0x107,

  OP_CB_ROTATE_LEFT_CIRCULAR_HL = 0x106,

  // - - - Cb rotate right 
  OP_CB_ROTATE_RIGHT_CIRCULAR_B = 0x108,
  OP_CB_ROTATE_RIGHT_CIRCULAR_C = 0x109,
  OP_CB_ROTATE_RIGHT_CIRCULAR_D = 0x10A,
  OP_CB_ROTATE_RIGHT_CIRCULAR_E = 0x10B,
  OP_CB_ROTATE_RIGHT_CIRCULAR_H = 0x10C,
  OP_CB_ROTATE_RIGHT_CIRCULAR_L = 0x10D,
  OP_CB_ROTATE_RIGHT_CIRCULAR_A = 0x10F,

  OP_CB_ROTATE_RIGHT_CIRCULAR_HL = 0x10E,

  // - - - Rotate left cb 
  OP_CB_ROTATE_LEFT_B = 0x110,
  OP_CB_ROTATE_LEFT_C = 0X111,
  OP_CB_ROTATE_LEFT_D = 0x112,
  OP_CB_ROTATE_LEFT_E = 0x113,
  OP_CB_ROTATE_LEFT_H = 0x114,
  OP_CB_ROTATE_LEFT_L = 0x115,
  OP_CB_ROTATE_LEFT_A = 0x117,

  OP_CB_ROTATE_LEFT_HL = 0x116,

  // - - - Rotate right cb 
  OP_CB_ROTATE_RIGHT_B = 0x118,
  OP_CB_ROTATE_RIGHT_C = 0x119,
  OP_CB_ROTATE_RIGHT_D = 0x11A,
  OP_CB_ROTATE_RIGHT_E = 0x11B,
  OP_CB_ROTATE_RIGHT_H = 0x11C,
  OP_CB_ROTATE_RIGHT_L = 0x11D,
  OP_CB_ROTATE_RIGHT_A = 0x11F,

  OP_CB_ROTATE_RIGHT_HL = 0x11E,

  // - - - Shift Left Arithmetic 
  OP_CB_SHIFT_LEFT_ARITH_B = 0x120,
  OP_CB_SHIFT_LEFT_ARITH_C = 0x121,
  OP_CB_SHIFT_LEFT_ARITH_D = 0x122,
  OP_CB_SHIFT_LEFT_ARITH_E = 0x123,
  OP_CB_SHIFT_LEFT_ARITH_H = 0x124,
  OP_CB_SHIFT_LEFT_ARITH_L = 0x125,
  OP_CB_SHIFT_LEFT_ARITH_A = 0x127,

  OP_CB_SHIFT_LEFT_ARITH_HL = 0x126,

  // - - - Shift Right Arithmetic 
  OP_CB_SHIFT_RIGHT_ARITH_B = 0x128,
  OP_CB_SHIFT_RIGHT_ARITH_C = 0x129,
  OP_CB_SHIFT_RIGHT_ARITH_D = 0x12A,
  OP_CB_SHIFT_RIGHT_ARITH_E = 0x12B, 
  OP_CB_SHIFT_RIGHT_ARITH_H = 0x12C,
  OP_CB_SHIFT_RIGHT_ARITH_L = 0x12D,
  OP_CB_SHIFT_RIGHT_ARITH_A = 0x12F,
  
  OP_CB_SHIFT_RIGHT_ARITH_HL = 0x12E,

  // - - - swap 
  OP_CB_SWAP_B = 0x130,
  OP_CB_SWAP_C = 0x131,
  OP_CB_SWAP_D = 0x132,
  OP_CB_SWAP_E = 0x133,
  OP_CB_SWAP_H = 0x134,
  OP_CB_SWAP_L = 0x135,
  OP_CB_SWAP_A = 0x137,
  
  OP_CB_SWAP_HL = 0x136,

  // - - - shift right logical 
  OP_CB_SHIFT_RIGHT_LOGIC_B = 0x138,
  OP_CB_SHIFT_RIGHT_LOGIC_C = 0x139,
  OP_CB_SHIFT_RIGHT_LOGIC_D = 0x13A,
  OP_CB_SHIFT_RIGHT_LOGIC_E = 0x13B,
  OP_CB_SHIFT_RIGHT_LOGIC_H = 0x13C,
  OP_CB_SHIFT_RIGHT_LOGIC_L = 0x13D,
  OP_CB_SHIFT_RIGHT_LOGIC_A = 0x13F,

  OP_CB_SHIFT_RIGHT_LOGIC_HL = 0x13E, 

  // - - - Bit read 
  OP_CB_BIT_0_B  = 0x140,
  OP_CB_BIT_0_C  = 0x141,
  OP_CB_BIT_0_D  = 0x142,
  OP_CB_BIT_0_E  = 0x143,
  OP_CB_BIT_0_H  = 0x144,
  OP_CB_BIT_0_L  = 0x145,
  OP_CB_BIT_0_A  = 0x147,

  OP_CB_BIT_0_HL = 0x146,
  
  OP_CB_BIT_1_B  = 0x148,
  OP_CB_BIT_1_C  = 0x149,
  OP_CB_BIT_1_D  = 0x14A,
  OP_CB_BIT_1_E  = 0x14B,
  OP_CB_BIT_1_H  = 0x14C,
  OP_CB_BIT_1_L  = 0x14D,
  OP_CB_BIT_1_A  = 0x14F,

  OP_CB_BIT_1_HL = 0x14E,

  OP_CB_BIT_2_B  = 0x150,
  OP_CB_BIT_2_C  = 0x151,
  OP_CB_BIT_2_D  = 0x152,
  OP_CB_BIT_2_E  = 0x153,
  OP_CB_BIT_2_H  = 0x154,
  OP_CB_BIT_2_L  = 0x155,
  OP_CB_BIT_2_A  = 0x157,

  OP_CB_BIT_2_HL = 0x156,

  OP_CB_BIT_3_B  = 0x158,
  OP_CB_BIT_3_C  = 0x159,
  OP_CB_BIT_3_D  = 0x15A,
  OP_CB_BIT_3_E  = 0x15B,
  OP_CB_BIT_3_H  = 0x15C,
  OP_CB_BIT_3_L  = 0x15D,
  OP_CB_BIT_3_A  = 0x15F,

  OP_CB_BIT_3_HL = 0x15E,

  OP_CB_BIT_4_B  = 0x160,
  OP_CB_BIT_4_C  = 0x161,
  OP_CB_BIT_4_D  = 0x162,
  OP_CB_BIT_4_E  = 0x163,
  OP_CB_BIT_4_H  = 0x164,
  OP_CB_BIT_4_L  = 0x165,
  OP_CB_BIT_4_A  = 0x167,

  OP_CB_BIT_4_HL = 0x166,

  OP_CB_BIT_5_B  = 0x168,
  OP_CB_BIT_5_C  = 0x169,
  OP_CB_BIT_5_D  = 0x16A,
  OP_CB_BIT_5_E  = 0x16B,
  OP_CB_BIT_5_H  = 0x16C,
  OP_CB_BIT_5_L  = 0x16D,
  OP_CB_BIT_5_A  = 0x16F,

  OP_CB_BIT_5_HL = 0x16E,

  OP_CB_BIT_6_B  = 0x170,
  OP_CB_BIT_6_C  = 0x171,
  OP_CB_BIT_6_D  = 0x172,
  OP_CB_BIT_6_E  = 0x173,
  OP_CB_BIT_6_H  = 0x174,
  OP_CB_BIT_6_L  = 0x175,
  OP_CB_BIT_6_A  = 0x177,

  OP_CB_BIT_6_HL = 0x176,

  OP_CB_BIT_7_B  = 0x178,
  OP_CB_BIT_7_C  = 0x179,
  OP_CB_BIT_7_D  = 0x17A,
  OP_CB_BIT_7_E  = 0x17B,
  OP_CB_BIT_7_H  = 0x17C,
  OP_CB_BIT_7_L  = 0x17D,
  OP_CB_BIT_7_A  = 0x17F,

  OP_CB_BIT_7_HL = 0x17E,

  // - - - bit reset 
  OP_CB_RESET_0_B = 0x180,
  OP_CB_RESET_0_C = 0x181,
  OP_CB_RESET_0_D = 0x182,
  OP_CB_RESET_0_E = 0x183,
  OP_CB_RESET_0_H = 0x184,
  OP_CB_RESET_0_L = 0x185,
  OP_CB_RESET_0_A = 0x187,

  OP_CB_RESET_0_HL = 0x186,

  OP_CB_RESET_1_B = 0x188,
  OP_CB_RESET_1_C = 0x189,
  OP_CB_RESET_1_D = 0x18A,
  OP_CB_RESET_1_E = 0x18B,
  OP_CB_RESET_1_H = 0x18C,
  OP_CB_RESET_1_L = 0x18D,
  OP_CB_RESET_1_A = 0x18F,

  OP_CB_RESET_1_HL = 0x18E,
  
  OP_CB_RESET_2_B = 0x190,
  OP_CB_RESET_2_C = 0x191,
  OP_CB_RESET_2_D = 0x192,
  OP_CB_RESET_2_E = 0x193,
  OP_CB_RESET_2_H = 0x194,
  OP_CB_RESET_2_L = 0x195,
  OP_CB_RESET_2_A = 0x197,

  OP_CB_RESET_2_HL = 0x196,

  OP_CB_RESET_3_B = 0x198,
  OP_CB_RESET_3_C = 0x199,
  OP_CB_RESET_3_D = 0x19A,
  OP_CB_RESET_3_E = 0x19B,
  OP_CB_RESET_3_H = 0x19C,
  OP_CB_RESET_3_L = 0x19D,
  OP_CB_RESET_3_A = 0x19F,

  OP_CB_RESET_3_HL = 0x19E,

  OP_CB_RESET_4_B = 0x1A0,
  OP_CB_RESET_4_C = 0x1A1,
  OP_CB_RESET_4_D = 0x1A2,
  OP_CB_RESET_4_E = 0x1A3,
  OP_CB_RESET_4_H = 0x1A4,
  OP_CB_RESET_4_L = 0x1A5,
  OP_CB_RESET_4_A = 0x1A7,

  OP_CB_RESET_4_HL = 0x1A6,
  
  OP_CB_RESET_5_B = 0x1A8,
  OP_CB_RESET_5_C = 0x1A9,
  OP_CB_RESET_5_D = 0x1AA,
  OP_CB_RESET_5_E = 0x1AB,
  OP_CB_RESET_5_H = 0x1AC,
  OP_CB_RESET_5_L = 0x1AD,
  OP_CB_RESET_5_A = 0x1AF,

  OP_CB_RESET_5_HL = 0x1AE,
  
  OP_CB_RESET_6_B = 0x1B0,
  OP_CB_RESET_6_C = 0x1B1,
  OP_CB_RESET_6_D = 0x1B2,
  OP_CB_RESET_6_E = 0x1B3,
  OP_CB_RESET_6_H = 0x1B4,
  OP_CB_RESET_6_L = 0x1B5,
  OP_CB_RESET_6_A = 0x1B7,

  OP_CB_RESET_6_HL = 0x1B6,

  OP_CB_RESET_7_B = 0x1B8,
  OP_CB_RESET_7_C = 0x1B9,
  OP_CB_RESET_7_D = 0x1BA,
  OP_CB_RESET_7_E = 0x1BB,
  OP_CB_RESET_7_H = 0x1BC,
  OP_CB_RESET_7_L = 0x1BD,
  OP_CB_RESET_7_A = 0x1BF,

  OP_CB_RESET_7_HL = 0x1BE,

  // - - - Bit set 
  OP_CB_SET_0_B = 0x1C0,
  OP_CB_SET_0_C = 0x1C1,
  OP_CB_SET_0_D = 0x1C2,
  OP_CB_SET_0_E = 0x1C3,
  OP_CB_SET_0_H = 0x1C4,
  OP_CB_SET_0_L = 0x1C5,
  OP_CB_SET_0_A = 0x1C7,

  OP_CB_SET_0_HL = 0x1C6,

  OP_CB_SET_1_B = 0x1C8,
  OP_CB_SET_1_C = 0x1C9,
  OP_CB_SET_1_D = 0x1CA,
  OP_CB_SET_1_E = 0x1CB,
  OP_CB_SET_1_H = 0x1CC,
  OP_CB_SET_1_L = 0x1CD,
  OP_CB_SET_1_A = 0x1CF,

  OP_CB_SET_1_HL = 0x1CE,
  
  OP_CB_SET_2_B = 0x1D0,
  OP_CB_SET_2_C = 0x1D1,
  OP_CB_SET_2_D = 0x1D2,
  OP_CB_SET_2_E = 0x1D3,
  OP_CB_SET_2_H = 0x1D4,
  OP_CB_SET_2_L = 0x1D5,
  OP_CB_SET_2_A = 0x1D7,

  OP_CB_SET_2_HL = 0x1D6,

  OP_CB_SET_3_B = 0x1D8,
  OP_CB_SET_3_C = 0x1D9,
  OP_CB_SET_3_D = 0x1DA,
  OP_CB_SET_3_E = 0x1DB,
  OP_CB_SET_3_H = 0x1DC,
  OP_CB_SET_3_L = 0x1DD,
  OP_CB_SET_3_A = 0x1DF,

  OP_CB_SET_3_HL = 0x1DE,

  OP_CB_SET_4_B = 0x1E0,
  OP_CB_SET_4_C = 0x1E1,
  OP_CB_SET_4_D = 0x1E2,
  OP_CB_SET_4_E = 0x1E3,
  OP_CB_SET_4_H = 0x1E4,
  OP_CB_SET_4_L = 0x1E5,
  OP_CB_SET_4_A = 0x1E7,

  OP_CB_SET_4_HL = 0x1E6,
  
  OP_CB_SET_5_B = 0x1E8,
  OP_CB_SET_5_C = 0x1E9,
  OP_CB_SET_5_D = 0x1EA,
  OP_CB_SET_5_E = 0x1EB,
  OP_CB_SET_5_H = 0x1EC,
  OP_CB_SET_5_L = 0x1ED,
  OP_CB_SET_5_A = 0x1EF,

  OP_CB_SET_5_HL = 0x1EE,
  
  OP_CB_SET_6_B = 0x1F0,
  OP_CB_SET_6_C = 0x1F1,
  OP_CB_SET_6_D = 0x1F2,
  OP_CB_SET_6_E = 0x1F3,
  OP_CB_SET_6_H = 0x1F4,
  OP_CB_SET_6_L = 0x1F5,
  OP_CB_SET_6_A = 0x1F7,

  OP_CB_SET_6_HL = 0x1F6,

  OP_CB_SET_7_B = 0x1F8,
  OP_CB_SET_7_C = 0x1F9,
  OP_CB_SET_7_D = 0x1FA,
  OP_CB_SET_7_E = 0x1FB,
  OP_CB_SET_7_H = 0x1FC,
  OP_CB_SET_7_L = 0x1FD,
  OP_CB_SET_7_A = 0x1FF,

  OP_CB_SET_7_HL = 0x1FE,

  // - - - Jump instructions 
  OP_JUMP_16_BIT_IMM            = 0xC3,
  OP_JUMP_HL                    = 0xE9,
  OP_JUMP_NZ_16_BIT_IMM         = 0xC2,
  OP_JUMP_Z_16_BIT_IMM          = 0xCA,
  OP_JUMP_NC_16_BIT_IMM         = 0xD2,
  OP_JUMP_C_16_BIT_IMM          = 0xDA,
  OP_JUMP_SIGNED_8_BIT_IMM      = 0x18,
  OP_JUMP_NZ_SIGNED_8_BIT_IMM   = 0x20,
  OP_JUMP_Z_SIGNED_8_BIT_IMM    = 0x28,
  OP_JUMP_NC_SIGNED_8_BIT_IMM   = 0x30,
  OP_JUMP_C_SIGNED_8_BIT_IMM    = 0x38,

  // - - - Call function 
  OP_CALL_16_BIT_IMM     = 0xCD,
  OP_CALL_NZ_16_BIT_IMM  = 0xC4,
  OP_CALL_Z_16_BIT_IMM   = 0xCC,
  OP_CALL_NC_16_BIT_IMM  = 0xD4,
  OP_CALL_C_16_BIT_IMM   = 0xDC,

  // - - - Return instructions 
  OP_RETURN           = 0xC9,
  OP_RETURN_NZ        = 0xC0,
  OP_RETURN_Z         = 0xC8,
  OP_RETURN_NC        = 0xD0,
  OP_RETURN_C         = 0xD8,
  OP_RETURN_INTERRUPT = 0xD9,

  // - - - Restart 
  OP_RESTART_00 = 0xC7,
  OP_RESTART_08 = 0xCF,
  OP_RESTART_10 = 0xD7,
  OP_RESTART_18 = 0xDF,
  OP_RESTART_20 = 0xE7,
  OP_RESTART_28 = 0xEF,
  OP_RESTART_30 = 0xF7,
  OP_RESTART_38 = 0xFF,

  // - - - Miscellanoeus
  OP_DISABLE_INTERRUPT  = 0xF3,
  OP_ENABLE_INTERRUPT   = 0xFB,
  OP_NOP                = 0x00,
  OP_STOP               = 0x10,
  OP_HALT               = 0x76,
} Opcode;


/**
 * @brief Instruction metadata record for a single opcode.
 *
 * This is *not* the decoded runtime state; it is static metadata used by decode/trace
 * and by generic handlers (e.g., LD r,d8 using reg1 + imm8).
*/
typedef struct Instruction
{
  ExecStatus (*handler)(void);  ///< M cycle step function
  AddressMode     mode;         ///< operand/addressing mode
  Opcode          opcode;       ///< Opcode of the instruction

  RegType         reg1;         ///< primary register operand (dest or first operand)
  RegType         reg2;         ///< secondary register operand (src or second operand)

  ConditionType   cond;         ///< branch condition, if applicable

  FlagPack        flags;        ///< packed Z/N/H/C behavior (2 bits each)

  u8              param;        ///< misc parameter (bit index, rst vector/8, etc.), family-defined
} Instruction;


// - - - Lookup API - - -

/// @brief initializes the instructionTable 
void instrctionTableInit(void);

/// @brief Lookup unprefixed opcode metadata.
const Instruction* instructionGetByOpcode(Opcode OPCODE);

/// @brief Lookup CB-prefixed opcode metadata (the byte after 0xCB).
const Instruction* instructionGetByCBOpcode(Opcode OPCODE);

/// @brief Get printable mnemonic name for an InstructionType.
const char* instructionGetName(Opcode TYPE);

/// @brief Get printable name for a register operand.
const char* instructionGetRegName(RegType REG);

#define INSTRUCTION_COUNT 256
