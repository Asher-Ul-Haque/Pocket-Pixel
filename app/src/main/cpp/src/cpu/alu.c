#include <cpu/alu.h>

/// @brief 8-bit Addition (Handles ADD, ADC).
AluResult8 aluAdd8(u8 A, u8 B, bool CARRY_IN) 
{
  AluResult8 res;

  u16 sum = (u16)A + (u16)B + (u16)(CARRY_IN ? 1 : 0);
  
  res.result   = (u8)(sum & 0xFF);
  res.zero     = (res.result == 0);
  res.negative = false;

  // - - - Half-carry: bit 3 overflow
  res.halfCarry = ((A & 0xF) + (B & 0xF) + (CARRY_IN ? 1 : 0)) > 0xF;
  res.carry     = (sum > 0xFF);
  
  return res;
}

/// @brief 8-bit Subtraction (Handles SUB, SBC, CP).
AluResult8 aluSub8(u8 A, u8 B, bool CARRY_IN) 
{
  AluResult8 res;

  i32 sub = (i32)A - (i32)B - (i32)(CARRY_IN ? 1 : 0);
  
  res.result    = (u8)(sub & 0xFF);
  res.zero      = (res.result == 0);
  res.negative  = true;

  // - - - Half-borrow: bit 4 borrow
  res.halfCarry = ((i32)(A & 0xF) - (i32)(B & 0xF) - (i32)(CARRY_IN ? 1 : 0)) < 0;
  res.carry     = (sub < 0);
  
  return res;
}

AluResult8 aluAnd8(u8 A, u8 B) 
{
  AluResult8 res;

  res.result    = A & B;
  res.zero      = (res.result == 0);
  res.negative  = false;
  res.halfCarry = true;  /// AND always sets H in SM83
  res.carry     = false;
  return res;
}

AluResult8 aluOr8(u8 A, u8 B)
{
  AluResult8 res;
  res.result    = A | B;
  res.zero      = (res.result == 0);
  res.negative  = false;
  res.halfCarry = false;
  res.carry     = false;
  return res;
}

AluResult8 aluXor8(u8 A, u8 B) 
{
  AluResult8 res;
  res.result    = A ^ B;
  res.zero      = (res.result == 0);
  res.negative  = false;
  res.halfCarry = false;
  res.carry     = false;
  return res;
}

AluResult8 aluInc8(u8 VAL) 
{
  AluResult8 res;
  res.result    = VAL + 1;
  res.zero      = (res.result == 0);
  res.negative  = false;
  res.halfCarry = (VAL & 0xF) == 0xF;
  res.carry     = false; /// INC does not affect Carry flag
  return res;
}

AluResult8 aluDec8(u8 VAL) 
{
  AluResult8 res;
  res.result    = VAL - 1;
  res.zero      = (res.result == 0);
  res.negative  = true;
  res.halfCarry = (VAL & 0xF) == 0;
  res.carry     = false; /// DEC does not affect Carry flag
  return res;
}

AluResult16 aluAdd16(u16 A, u16 B) 
{
  AluResult16 res;

  u32 sum = (u32)A + (u32)B;

  res.result    = (u16)(sum & 0xFFFF);
  res.zero      = false; /// 16-bit ADD does not affect Z
  res.negative  = false;
  res.halfCarry = ((A & 0x0FFF) + (B & 0x0FFF)) > 0x0FFF; /// Carry from bit 11
  res.carry     = (sum > 0xFFFF);
  return res;
}

AluResult16 aluAdd16Sp(u16 SP, i8 REL) 
{
  AluResult16 res;
  res.result = SP + REL;
  
  u32 a = (u32)SP;
  u32 b = (u32)(u8)REL; 

  res.zero      = false; // - - - Instruction always resets Z
  res.negative  = false; // - - - Instruction always resets N
  res.halfCarry = ((a & 0xF)  + (b & 0xF))  > 0xF;
  res.carry     = ((a & 0xFF) + (b & 0xFF)) > 0xFF;
  
  return res;
}

AluResult8 aluDaa(u8 A, bool NEGATIVE, bool HALF_CARRY, bool CARRY) 
{
  u8   correction = 0;
  bool setC       = false;

  if (HALF_CARRY || (!NEGATIVE && (A & 0xF) > 9)) 
  { correction |= 0x06; }

  if (CARRY || (!NEGATIVE && A > 0x99)) 
  {
    correction |= 0x60;
    setC = true; 
  }

  u8 res = NEGATIVE ? (A - correction) : (A + correction);

  return (AluResult8)
    {
      .result     = res,
      .zero       = (res == 0),
      .negative   = NEGATIVE,
      .halfCarry  = false,
      .carry      = setC
    };
}
