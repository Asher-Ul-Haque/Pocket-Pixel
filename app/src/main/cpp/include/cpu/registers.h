/**
 * @file registers.h
 * This file defines the register file for the sm83 game boy CPU.
 * The sm83 has 8 8-bit registers (a, f, b, c, d, e, h, l) and 2 16-bit registers (stack pointer and program counter).
 * The flag register (f) contains the following bits:
 *  - Bit 7: Zero Flag (Z)
 *  - Bit 6: Subtract Flag (N)
 *  - Bit 5: Half Carry Flag (H)
 *  - Bit 4: Carry Flag (C)
 * Most 8 bit registers can also be accessed as 16 bit registers by combining them in pairs:
 * - AF: a and f
 * - BC: b and c
 * - DE: d and e
 * - HL: h and l
 *
 * The Zero Flag (Z) is set when the result of an operation is zero. Used by conditional jumps
 * The Carry Flag (C):
 *  - When result of an 8-bit addition is higher than 0XFF.
 *  - When result of a 16-bit addition is higher than 0XFFFF.
 *  - When the result of a subtraction is negative 
 *  - When a shift operation shifts out a 1 bit.
 * Used by conditional jumps and instructions such as ADC, SBC, RL, RLA, etc.
*/

#pragma once 
#include <common.h>

///@ brief The register file for the sm83 CPU.
typedef struct RegisterFile
{
  // - - - 8 bit registers - - -
  u8 a; ///< accumulator register
  u8 f; ///< flag register for storing the status of various operations (Z, N, H, C)
        
  // - - - general purpose registers - - -
  u8 b; ///< general purpose register
  u8 c; ///< general purpose register
  u8 d; ///< general purpose register
  u8 e; ///< general purpose register
  u8 h; ///< general purpose register
  u8 l; ///< general purpose register

  // - - - 16 bit registers - - -
  u16 stackPointer;    ///< stack pointer register for function calls and interrupts
  u16 programCounter;  ///< program counter register for keeping track of the current instruction being executed
} RegisterFile;

#define CPU_FLAG_Z_MASK (1u << 7) ///< Mask for the Zero Flag (Z) in the flag register
#define CPU_FLAG_N_MASK (1u << 6) ///< Mask for the Subtract Flag (N) in the flag register
#define CPU_FLAG_H_MASK (1u << 5) ///< Mask for the Half Carry Flag (H) in the flag register
#define CPU_FLAG_C_MASK (1u << 4) ///< Mask for the Carry Flag (C) in the flag register
                                  
#define CPU_FLAG_ZERO_GET   BIT(CTX->registers.f, 7) ///< Macro to access the Zero Flag (Z) in the flag register
#define CPU_FLAG_SUB_GET    BIT(CTX->registers.f, 6) ///< Macro to access the Subtract Flag (N) in the flag register
#define CPU_FLAG_HALF_GET   BIT(CTX->registers.f, 5) ///< Macro to access the Half Carry Flag (H) in the flag register
#define CPU_FLAG_CARRY_GET  BIT(CTX->registers.f, 4) ///< Macro to access the Carry Flag (C) in the flag register

#define CPU_FLAG_ZERO_SET(ON)   BIT_SET(CTX->registers.f, 7, ON) ///< Macro to set or clear the Zero Flag (Z) in the flag register
#define CPU_FLAG_SUB_SET(ON)    BIT_SET(CTX->registers.f, 6, ON) ///< Macro to set or clear the Subtract Flag (N) in the flag register
#define CPU_FLAG_HALF_SET(ON)   BIT_SET(CTX->registers.f, 5, ON) ///< Macro to set or clear the Half Carry Flag (H) in the flag register
#define CPU_FLAG_CARRY_SET(ON)  BIT_SET(CTX->registers.f, 4, ON) ///< Macro to set or clear the Carry Flag (C) in the flag register
                                                            
#define CPU_REG_AF ((u16*) &CTX->registers.a) ///< Macro to access the combined AF register (A and F)
#define CPU_REG_BC ((u16*) &CTX->registers.b) ///< Macro to access the combined BC register (B and C)
#define CPU_REG_DE ((u16*) &CTX->registers.d)  ///< Macro to access the combined DE register (D and E)
#define CPU_REG_HL ((u16*) &CTX->registers.h) ///< Macro to access the combined HL register (H and L)                                         
