#pragma once 

/**
 * @file cpu.h
 * @brief SM83 CPU core public API (DMG + CGB).
 *
 * This module intentionally does not embed a "model" field. The model is read from
 * the loaded cartridge via CartContext.mode (MODE_DMG_GAMEBOY / MODE_CGB_GAMEBOY / MODE_CGB_ONLY_GAMEBOY).
 */

#include <cpu/interrupts.h>
#include <common.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>


/**
 * @brief Represents the state of the CPU, including registers, flags, and other relevant information.
*/
typedef struct CpuContext 
{
  RegisterFile registers; ///< Registers 

  // - - - Execution state 
  Opcode              currentOpcode;      ///< Current opcode 
  const Instruction*  currentInstruction; ///< current instruction pointer
  u8                  mCycle;             ///< current m cycle
  bool                isCB;               ///< are we in cb table
  CpuInterrupt        servicingInt;       ///< Current interrupt being serviced

  // - - - Internal Latches 
  u8  latchedVal8;    ///< Internal bus data latch
  u16 latchedAddr16;  ///< Internal address bus latch
  u16 pcAtFetch;      ///< Track where instruction started

  // - - - System and CGB flags
  bool ime;         ///< Interrupt Master Enable
  bool imeDelay;    ///< EI delay logic
  bool halted;      ///< Is the cpu halted
  bool haltBug;     ///< Halt bug latch
  bool stopped;     ///< Is the cpu stopped
  bool doubleSpeed; ///< CGB Gear: affects Timer and Dispatcher timing


  u64  totalMCycles; ///< how many cycles has the cpu done
} CpuContext;


// - - - CPU function - - -

/**
 * @brief Retrieves a pointer to teh current CPU Context 
 * @return A pointer to the current CpuContext structure
 * @see CpuContext 
 * @warning This function should be used with caution, as it provides direct access to the internal state of the CPU. Modifying the returned CpuContext can lead to unintended consequences if not done carefully.
*/
CpuContext* cpuGetContext(void);

/** 
 * @brief Initializes the CPU by setting up the initial state of the registers, flags, and other relevant information. This function should be called before starting the emulation process to ensure that the CPU is in a known and consistent state.
 * @warning This must be called after loading a cartridge
*/
void cpuInit(void);

/// @ brief Perform 1 M cycle
void cpuTick(void);

/**
 * @brief Produce a one-line trace (PC/opcode bytes + regs + flags).
 * @param OUT Output buffer to write the trace string into.
 * @param OUT_SIZE Size of the output buffer in bytes.
 * @warn Intended for trace/debug. Uses ctx->pcAtFetch, ctx->opcode,
*/
void cpuTraceLineToString(char* OUT, u32 OUT_SIZE);

#define START_VALUE_PROGRAM_COUNTER 0x100u
#define START_VALUE_STACK_POINTER   0xFFFEu

#define START_VALUE_AF_DMG 0x01B0u
#define START_VALUE_BC_DMG 0x0013u
#define START_VALUE_DE_DMG 0x00D8u
#define START_VALUE_HL_DMG 0x014Du

#define START_VALUE_AF_CGB 0x11B0u
#define START_VALUE_BC_CGB 0x0000u
#define START_VALUE_DE_CGB 0xFF56u
#define START_VALUE_HL_CGB 0x000Du

#define M1 0 
#define M2 1 
#define M3 2 
#define M4 3 
#define M5 4 
#define M6 5 
#define M7 6
