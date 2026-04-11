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
#include <cpu/instructions.h>

/**
 * @brief Represents the state of the CPU, including registers, flags, and other relevant information.
*/
typedef struct CpuContext 
{
  bool          doubleSpeed; ///< Indicates if the CPU is operating in double speed mode (CGB feature).

  RegisterFile  registers;

  // - - - data extracted during decode
  u16                 readData;
  u16                 memDest;
  bool                destIsMem;
  bool                isCB;
  u8                  currentOpcode;
  u8                  cbOpcode;
  const Instruction*  currentInstruction;

  // - - - control flow
  bool halted;
  bool stopped;
  bool haltBug; ///< Set when an instruction is executed while the CPU is halted. Causes the next opcode fetch to read the same byte twice (PC not advanced properly). SameBoy models this by decrementing PC when haltBug is set.

  // - - - interrupts
  bool  interruptMasterEnabled;
  bool  enablingIme;
  u8    interrupt;
  u8    interruptFlags;

  // - - - data extracted during fetch
  u16 imm16;
  u8  imm8;
} CpuContext;


// - - - CPU function - - -

/**
 * @brief Retrieves a pointer to teh current CPU Context 
 * @return A pointer to the current CpuContext structure
 * @see CpuContext 
 * @warning This function should be used with caution, as it provides direct access to the internal state of the CPU. Modifying the returned CpuContext can lead to unintended consequences if not done carefully.
*/
CpuContext* cpuGetContext(void);

/// @brief Initializes the CPU by setting up the initial state of the registers, flags, and other relevant information. This function should be called before starting the emulation process to ensure that the CPU is in a known and consistent state.
void cpuInit(void);

/**
 * @brief Executes a single CPU tick, which involves fetching, decoding, and executing the next instruction based on the current state of the CPU. This function should be called repeatedly in a loop to simulate the continuous operation of the CPU.
 * @return A boolean value indicating whether the CPU tick was executed successfully. If the function returns false, it may indicate that an error occurred during instruction execution or that the CPU is in a halted.
*/
bool cpuTick(void);


/// @brief Fetch opcode at PC and populate ctx->opcode/ctx->inst/etc. Advances PC appropriately. 
void cpuFetchAndDecode(void);


/**
 * @brief Executes the instruction currently stored in the CPU context's currentInstruction field. This function assumes that the instruction has already been decoded and that all necessary information (such as operands and addressing modes) has been extracted during the decode phase. The execution of the instruction may involve manipulating registers, memory, flags, and other aspects of the CPU state based on the specific operation defined by the instruction.
*/
void cpuExecDecoded(void);

/**
 * @brief Converts the current instruction stored in the CPU context into a human-readable string format. This function is useful for debugging and logging purposes, allowing developers to see a textual representation of the instruction being executed. The resulting string is stored in the provided output buffer, and the size of the buffer is specified to prevent overflow.
 * @param OUT A pointer to a character array (string) where the resulting instruction string will be stored.
 * @param OUT_SIZE The size of the output buffer (OUT) to ensure that the function
*/
void cpuInstructionToString(char* OUT, u32 OUT_SIZE);

/**
 * @brief Produce a one-line trace similar in spirit to SameBoy (PC/opcode bytes + regs + flags).
 * @param PC_AT_FETCH PC before fetching the opcode.
 * @param OUT Output buffer for the resulting string.
 * @param OUT_SIZE Size of the output buffer.
 */
void cpuTraceLineToString(u16 PC_AT_FETCH, char* OUT, u32 OUT_SIZE);


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

