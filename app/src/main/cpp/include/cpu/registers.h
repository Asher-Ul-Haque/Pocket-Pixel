#pragma once
/**
 * @file registers.h
 * @brief SM83 register file and flag definitions.
 *
 * This header defines:
 * - The CPU architectural registers (A,F,B,C,D,E,H,L,SP,PC)
 * - Flag bit masks and small helpers for reading/writing flags
 *
 * Notes:
 * - The F register lower nibble is always 0 on real hardware.
 * - This header intentionally does not include cpu.h to avoid circular deps.
 */

#include <common.h>
/**
 * @brief CPU architectural register file.
 *
 * 8-bit registers are stored as separate bytes for clarity and portability.
 * 16-bit registers (SP/PC) are stored explicitly.
*/
typedef struct RegisterFile
{
  // - - - 8-bit registers
  u8 a;
  u8 f;

  u8 b;
  u8 c;

  u8 d;
  u8 e;

  u8 h;
  u8 l;

  // - - -  16-bit registers
  u16 stackPointer;
  u16 programCounter;
} RegisterFile;


// - - - Flag bit masks - - - 

// - - - Zero flag (Z) bit mask in F.
#define CPU_FLAG_ZERO_MASK (0x80u)

// - - - Subtract flag (N) bit mask in F.
#define CPU_FLAG_NEGATIVE_MASK (0x40u)

// - - - Half-carry flag (H) bit mask in F.
#define CPU_FLAG_HALF_CARRY_MASK (0x20u)

// - - - Carry flag (C) bit mask in F.
#define CPU_FLAG_CARRY_MASK (0x10u)

// - - - Lower nibble of F is always 0.
#define CPU_FLAG_LOW_MASK (0x0Fu)

// - - - Mask for all valid flag bits.
#define CPU_FLAG_VALID_MASK (0xF0u)


// - - - 16-bit pack/unpack helpers - - - 

static inline u16 cpuMakeU16(u8 HIGH, u8 LOW) { return (u16)(((u16)HIGH << 8) | (u16)LOW); }
static inline u8  cpuHi8(u16 VALUE)           { return (u8)(VALUE >> 8); }
static inline u8  cpuLo8(u16 VALUE)           { return (u8)(VALUE & 0xFFu); }


// - - - Register pair helpers - - - 

static inline u16 cpuGetAF(const RegisterFile* REGISTERS) { return cpuMakeU16(REGISTERS->a, (u8)(REGISTERS->f & CPU_FLAG_VALID_MASK)); }
static inline u16 cpuGetBC(const RegisterFile* REGISTERS) { return cpuMakeU16(REGISTERS->b, REGISTERS->c); }
static inline u16 cpuGetDE(const RegisterFile* REGISTERS) { return cpuMakeU16(REGISTERS->d, REGISTERS->e); }
static inline u16 cpuGetHL(const RegisterFile* REGISTERS) { return cpuMakeU16(REGISTERS->h, REGISTERS->l); }

static inline void cpuSetAF(RegisterFile* REGISTERS, u16 VALUE) { REGISTERS->a = cpuHi8(VALUE); REGISTERS->f = (u8)(cpuLo8(VALUE) & CPU_FLAG_VALID_MASK); }
static inline void cpuSetBC(RegisterFile* REGISTERS, u16 VALUE) { REGISTERS->b = cpuHi8(VALUE); REGISTERS->c = cpuLo8(VALUE); }
static inline void cpuSetDE(RegisterFile* REGISTERS, u16 VALUE) { REGISTERS->d = cpuHi8(VALUE); REGISTERS->e = cpuLo8(VALUE); }
static inline void cpuSetHL(RegisterFile* REGISTERS, u16 VALUE) { REGISTERS->h = cpuHi8(VALUE); REGISTERS->l = cpuLo8(VALUE); }


// - - - Flag read helpers - - - 

static inline bool cpuFlagZ(const RegisterFile* r) { return (r->f & CPU_FLAG_ZERO_MASK) != 0; }
static inline bool cpuFlagN(const RegisterFile* r) { return (r->f & CPU_FLAG_NEGATIVE_MASK) != 0; }
static inline bool cpuFlagH(const RegisterFile* r) { return (r->f & CPU_FLAG_HALF_CARRY_MASK) != 0; }
static inline bool cpuFlagC(const RegisterFile* r) { return (r->f & CPU_FLAG_CARRY_MASK) != 0; }


// - - - Flag write helper - - -

static inline void cpuFlagSet(RegisterFile* REGISTER_FILE, u8 MASK, bool ON)
{
  if (ON) REGISTER_FILE->f = (u8)(REGISTER_FILE->f | MASK);
  else    REGISTER_FILE->f = (u8)(REGISTER_FILE->f & (u8)~MASK);

  REGISTER_FILE->f &= CPU_FLAG_VALID_MASK;
}

static inline void cpuSetZ(RegisterFile* REGISTERS, bool ON) { cpuFlagSet(REGISTERS, CPU_FLAG_ZERO_MASK, ON); }
static inline void cpuSetN(RegisterFile* REGISTERS, bool ON) { cpuFlagSet(REGISTERS, CPU_FLAG_NEGATIVE_MASK, ON); }
static inline void cpuSetH(RegisterFile* REGISTERS, bool ON) { cpuFlagSet(REGISTERS, CPU_FLAG_HALF_CARRY_MASK, ON); }
static inline void cpuSetC(RegisterFile* REGISTERS, bool ON) { cpuFlagSet(REGISTERS, CPU_FLAG_CARRY_MASK, ON); }

/// @brief Clear all flags (sets F=0).
static inline void cpuClearFlags(RegisterFile* REGISTER_FILE)
{ REGISTER_FILE->f = 0; }
