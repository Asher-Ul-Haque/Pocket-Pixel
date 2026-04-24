#pragma once
/**
 * @file ops.h
 * @brief Per-instruction-family execution entry points (M-cycle stepping).
 *
 * The CPU core executes instructions as a state machine. During CPU_STATE_EXECUTE,
 * cpu_execute.c will call exactly one of these functions each M-cycle based on the
 * currently decoded instruction type (ctx->info->type) and/or CB prefix state.
 *
 * Each ops*Step() function must:
 * - read the current CpuContext via cpuGetContext()
 * - perform at most the work for ONE M-cycle
 * - advance ctx->microState / ctx->mCycleInInstr as needed
 * - mark the instruction complete by transitioning ctx->state back to FETCH
 *   (or by calling a common "cpuFinishInstruction()" helper—implementation-defined)
 *
 * No other subsystems (PPU/APU/Timer/DMA) are ticked here.
*/

#include <common.h>

/// @brief Execute one M-cycle for non-CB CPU control/system instructions.
void opsControlStep(void);

/// @brief Execute one M-cycle for jump/call/return/rst instructions.
void opsJumpStep(void);

/// @brief Execute one M-cycle for load/store instructions.
void opsLoadStep(void);

/** * @brief Handle Opcode 0x08: LD (a16), SP.
 * Separated from generic LoadStep for cleaner dispatch.
 */
void opsLoadSpToAddrStep(void);

/**
 * @brief Handle High RAM/IO Loads (LDH).
 * Covers 0xE0, 0xF0, 0xE2, 0xF2.
 */
void opsLoadHighStep(void);

/// @brief Execute one M-cycle for 8-bit ALU instructions.
void opsAlu8Step(void);

/// @brief Execute one M-cycle for 16-bit ALU instructions (ADD HL, rr).
void opsAlu16Step(void);

/**
 * @brief Handle 16-bit SP arithmetic specials.
 * Covers Opcode 0xE8 (ADD SP, e8) and 0xF8 (LD HL, SP+e8).
 */
void opsAlu16SpecialStep(void);

/// @brief Execute one M-cycle for INC/DEC instructions (8-bit and 16-bit).
void opsIncDecStep(void);

/**
 * @brief Execute one M-cycle for CB-prefixed rotate/shift/swap group.
*/
void opsCbRotateShiftStep(void);

/**
 * @brief Execute one M-cycle for CB-prefixed bit manipulation group.
*/
void opsCbBitStep(void);

/**
 * @brief Handle rotate instructions
*/
void opsRotateStep(void);

/**
 * @brief Hanldes PUSH rr (4 M-cycles)
 * M1: Fetch, M2: Internal, M3: write Hi, M4: write Lo
*/
void opsPushStep(void);

/**
 * @brief Handles POP rr (3 M-cycles)
 * M1: Fetch, M2: read Lo, M3: read Hi
*/
void opsPopStep(void);
