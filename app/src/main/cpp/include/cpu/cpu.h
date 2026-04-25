#pragma once 

/**
 * @file cpu.h
 * @brief SM83 CPU core public API (DMG + CGB).
 *
 * This module intentionally does not embed a "model" field. The model is read from
 * the loaded cartridge via CartContext.mode (MODE_DMG_GAMEBOY / MODE_CGB_GAMEBOY / MODE_CGB_ONLY_GAMEBOY).
 */

#include <common.h>
#include <cpu/registers.h>
#include <cpu/instruction.h>


/**
 * @brief CPU top-level state machine states.
 * The CPU runs one M-cycle per cpuTickMCycle() call
*/
typedef enum CpuState 
{
  CPU_STATE_FETCH           = 0,  ///< Fetch unprefixed opcode at PC,
  CPU_STATE_FETCH_CB,             ///< Fetch CB-prefixed opcode at PC (after fetching 0xCB)
  CPU_STATE_DECODE,               ///< Decode the fetched opcode and extract relevant information (operands, addressing mode, etc.)                    
  CPU_STATE_EXECUTE,              ///< Execute the decoded instruction.
  CPU_STATE_INTERRUPT_ENTRY,      ///< Interrupt entry sequence (push PC, set vector, etc.)                      
} CpuState;


/**
 * @brief Represents the state of the CPU, including registers, flags, and other relevant information.
*/
typedef struct CpuContext 
{
  // - - - Architectural state - - - 

  RegisterFile  registers; ///< CPU registers
  
  bool halted;  ///< HALT state
  bool stopped; ///< STOP state
  bool haltBug; ///< HALT bug latch (when trigger, next fetch repeats byte)

  bool ime;        ///< Interrupt Master Enable 
  bool imePending; ///< EI delay latch (IME becomes true after next instruction complete)
                   
  bool doubleSpeed;  ///< CGB double-speed mode latch (KEY1 handling lives elsewhere)
  u64  mCyclesTotal; ///< Total M-cycles executed since power on (for profiling)                    

  CpuState state; ///< Current CPU state (fetch, decode, execute, etc.)
    

  // - - - Current Instruction - - - 

  u16  pcAtFetch; ///< PC value before fetching the opcode, used for tracing and debugging purposes.
  u8   opcode;    ///< Unprefixed opcode byte
  bool isCB;      ///< Whether current instruction is CB-prefixed
  u8   cbOpcode;  ///< CB opcode byte (valid if isCB)

  const Instruction*  instr;   ///< Decoded instruction metadata
  u8                  imm8;   ///< 8-bit immediate value (valid if instruction has an 8-bit immediate operand)
  u16                 imm16;  ///< 16-bit immediate value (valid if instruction has a 16-bit immediate operand)           

  u8 mCycleInInstr; ///< Which M-cycle we are in for the current instruction
  u8 microState;    ///< Family specfic microstate (interpreted by different modules)                    

  u16  addr;        ///< Effective address for memory operands
  u16  readData;    ///< Latched read value (8 or 16, stored in low bits)
  bool hasAddr;     
  bool hasReadData;

  bool conditionPassed;  ///< For conditional branches/calls/rets (affects cycles)
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

/// @brief Reset transient decode/execute state without changing cartridge-derived model selection.
void cpuReset(void);

/**
 * @brief Tick the CPU forward by exactly one M-cycle of work.
 * @note This function does NOT tick PPU/APU/timer/DMA. A higher-level scheduler must do that.
*/
void cpuStepMCycle(void);

/**
 * @brief Executes a single CPU tick, which involves fetching, decoding, and executing the next instruction based on the current state of the CPU. This function should be called repeatedly in a loop to simulate the continuous operation of the CPU.
 * @note This function does fetch decode execute all in one call
*/
void cpuTick(void);


/// @brief Fetches the next opcode from memory based on the current program counter (PC) and updates the CPU context with the fetched opcode and any relevant information. This function is responsible for advancing the PC and preparing the CPU for the subsequent decode and execute steps.
void cpuDecodeStep(void);


/// @brief Executes the currently decoded instruction based on the information stored in the CPU context. This function performs the necessary operations to carry out the instruction's behavior, including manipulating registers, memory, and flags as required by the instruction's semantics.
void cpuExecuteStep(void);


/// @brief Performs a single step of the CPU's operation, which includes fetching, decoding, and executing an instruction. This function is intended for tracing and debugging purposes, allowing developers to observe the CPU's behavior on a per-instruction basis. It may also include additional logging or state output to facilitate debugging.
void cpuTraceStep(void);

/// @brief Finish current instructiona nd return CPU to FETCH state. 
void cpuFinishInstruction(void);

// @brief Evaluate an Instruction condition (NZ/Z/NC/C) against current flags.
bool cpuEvalCond(ConditionType COND);

/**
 * @brief Convert the currently decoded instruction into a human-readable string.
 * @warn Intended for trace/debug. Uses ctx->info + ctx->imm8/imm16 (prefetched during decode).
 * @param OUT Output buffer to write the instruction string into.
 * @param OUT_SIZE Size of the output buffer in bytes.
*/
void cpuInstructionToString(char* OUT, u32 OUT_SIZE);

/**
 * @brief Produce a one-line trace (PC/opcode bytes + regs + flags).
 * @param PC_AT_FETCH PC before fetching opcode (ctx->pcAtFetch).
 * @param OUT Output buffer to write the trace string into.
 * @param OUT_SIZE Size of the output buffer in bytes.
 * @warn Intended for trace/debug. Uses ctx->pcAtFetch, ctx->opcode,
*/
void cpuTraceLineToString(u16 PC_AT_FETCH, char* OUT, u32 OUT_SIZE);


/**
 * @brief Stack helpers: cycle-stepped by caller using microState.
 * SM83 stack grows downward. Push writes high then low as SP decrements. 
*/
void cpuStackWriteHi(u16 VALUE);
void cpuStackWriteLo(u16 VALUE);
u8   cpuStackReadHi (void);
u8   cpuStackReadLo (void);


// - - - cpu flags

#define CPU_FLAG_Z BIT(CTX->regs.flags, 7)
#define CPU_FLAG_N BIT(CTX->regs.flags, 6)
#define CPU_FLAG_H BIT(CTX->regs.flags, 5)
#define CPU_FLAG_C BIT(CTX->regs.flags, 4)

// - - - Values - - - 

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

#define SPEED_SWITCH_ADDR 0xFF4Du
