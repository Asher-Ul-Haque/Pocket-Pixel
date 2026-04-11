/**
 * @file cpuOpcodeHandlers.h
 * @brief This file contains the declarations of the opcode handler functions for the CPU emulator.
 * Each function corresponds to a specific opcode and implements the behavior of that opcode when executed.
*/

#pragma once 
#include <common.h>

///@brief Executes the instruction currently stored in ctx->currentInstruction, which should have been populated by a prior call to cpuFetchAndDecode. This function performs the actual operations defined by the instruction, such as manipulating registers, memory, and flags, and may also handle control flow changes like jumps and calls.
typedef void (*CpuOpcodeHandler)(u8 OPCODE);

/**
 * @brief Retrieves a function pointer to the handler responsible for executing the instruction corresponding to the given opcode. The returned function pointer can be used to execute the instruction associated with the provided opcode by passing the current CPU context and the opcode as arguments.
 * @param OPCODE An 8-bit unsigned integer representing the opcode for which the handler function is to be retrieved.
 * @return A function pointer of type CpuOpcodeHandler that corresponds to the specified opcode. This function pointer can be invoked to execute the instruction associated with the given opcode.
*/
CpuOpcodeHandler cpuGetOpcodeHandler(u8 OPCODE);
CpuOpcodeHandler cpuGetCBOpcodeHandler(u8 CB_OPCODE);

void opJP_a16       (u8 OPCODE);
void opJP_HL        (u8 OPCODE);
void opJR_r8        (u8 OPCODE);
void opJR_cc_r8     (u8 OPCODE);
void opCALL_a16     (u8 OPCODE);
void opCALL_cc_a16  (u8 OPCODE);
void opRET          (u8 OPCODE);
void opRET_cc       (u8 OPCODE);
void opRETI         (u8 OPCODE);
void opRST          (u8 OPCODE);
void opDI           (u8 OPCODE);
void opEI           (u8 OPCODE);
void opHALT         (u8 OPCODE);
void opSTOP         (u8 OPCODE);
void opILLEGAL      (u8 OPCODE);
void opNOP          (u8 OPCODE);
