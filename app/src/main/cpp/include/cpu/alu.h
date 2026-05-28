/**
 * @file alu.h 
 * @brief Handles ALU operations safetly
*/
#pragma once
#include <common.h>

/**
 * @struct AluResult8
 * @brief Result of an 8-bit ALU operation including potential flags
*/
typedef struct 
{
  u8   result;
  bool zero;
  bool negative;
  bool halfCarry;
  bool carry;
} AluResult8;

/**
 * @struct AluResult16
 * @brief Result of an 16-bit ALU operation including potential flags
*/
typedef struct 
{
  u16  result;
  bool zero;
  bool negative;
  bool halfCarry;
  bool carry;
} AluResult16;


// - - - 8-bit Arithmetic - - - 

AluResult8 aluAdd8(u8 A, u8 B, bool CARRY_IN);
AluResult8 aluSub8(u8 A, u8 B, bool CARRY_IN); ///< Used for SUB, SBC, and CP
AluResult8 aluAnd8(u8 A, u8 B);
AluResult8 aluOr8 (u8 A, u8 B);
AluResult8 aluXor8(u8 A, u8 B);
AluResult8 aluInc8(u8 VAL);
AluResult8 aluDec8(u8 VAL);


// - - - 16-bit Arithmetic - - -

AluResult16 aluAdd16  (u16 A, u16 B);
AluResult16 aluAdd16Sp(u16 SP, i8 REL); ///< Special case for ADD SP, e8


// - - - Bitwise/Shifts (CB) - - -

AluResult8 aluRlc8 (u8 VAL);
AluResult8 aluRrc8 (u8 VAL);
AluResult8 aluRl8  (u8 VAL, bool CARRY_IN);
AluResult8 aluRr8  (u8 VAL, bool CARRY_IN);
AluResult8 aluSla8 (u8 VAL);
AluResult8 aluSra8 (u8 VAL);
AluResult8 aluSwap8(u8 VAL);
AluResult8 aluSrl8 (u8 VAL);

// - - - BCD - - -

AluResult8 aluDaa(u8 VAL, bool NEGATIVE, bool HALF_CARRY, bool CARRY);
