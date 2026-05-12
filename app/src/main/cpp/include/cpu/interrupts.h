#pragma once
/**
 * @file interrupts.h
 * @brief Interrupt definitions + interrupt entry stepping for the SM83 CPU.
 *
 * This header is part of the rewritten CPU module.
 *
 * Design goals:
 * - Keep interrupt semantics centralized (HALT behavior, IME rules, IF/IE bits, vectors).
 * - Provide a single stepping entry point for the interrupt-entry microsequence.
 *
 * Note:
 * - The actual IE/IF storage lives in the IO subsystem (typically at 0xFFFF and 0xFF0F).
 *   CPU code should call into io/interrupt register helpers (implemented elsewhere).
*/

#include <common.h>

/// @brief Interrupt bit indices (IE/IF bits) 
typedef enum CpuInterrupt
{
  CPU_INT_VBLANK = 0,
  CPU_INT_LCD    = 1,
  CPU_INT_TIMER  = 2,
  CPU_INT_SERIAL = 3,
  CPU_INT_JOYPAD = 4,
  CPUT_INT_NONE  = 0xFF
} CpuInterrupt;

// - - - Interrupt vectors
#define CPU_INT_VEC_VBLANK (0x40u)
#define CPU_INT_VEC_LCD    (0x48u)
#define CPU_INT_VEC_TIMER  (0x50u)
#define CPU_INT_VEC_SERIAL (0x58u)
#define CPU_INT_VEC_JOYPAD (0x60u)

typedef struct InterruptContext
{
  u8 interruptEnable;
  u8 interruptFlag;
} InterruptContext;

/// @brief Get the current interrupt context (IE and IF values).
InterruptContext* cpuInterruptGetContext(void);

/**
 * @brief Returns true if any interrupt is pending (IE & IF != 0).
 *
 * Used for:
 * - deciding whether HALT should wake
 * - deciding whether CPU should enter interrupt sequence when IME=1
*/
bool cpuInterruptPending(void);

/**
 * @brief Returns the highest-priority pending interrupt bit (0..4), and true if found.
 * Priority: VBLANK, LCD, TIMER, SERIAL, JOYPAD.
*/
CpuInterrupt cpuInterruptGetHighest(void);

/**
 * @brief Acknowledge/clear a specific interrupt request bit in IF.
 * @note This should clear IF bit, not IE.
*/
void cpuInterruptAcknowledge(CpuInterrupt INT);

/**
 * @brief Request an interrupt by setting the appropriate bit in IF.
 * @note This should set IF bit, not IE.
*/
void cpuRequestInterrupt(CpuInterrupt INT);

/// @brief Convert interrupt to vector address.
u16 cpuInterruptVector(CpuInterrupt INT);

/**
 * @brief Read the interrupt flag or enable register (IE or IF) via the BUS's perspective.
 * @param ADDRESS should be either 0xFF0F (IF) or 0xFFFF (IE).
*/
u8 cpuReadInterrupt(u16 ADDRESS);

/**
 * @brief Write the interrupt flag or enable register (IE or IF) via the BUS's perspective.
 * @param ADDRESS should be either 0xFF0F (IF) or 0xFFFF (IE).
 * @param VALUE is the value to write to the register (typically, VALUE will be a bitmask of CpuInterrupt bits).
*/
void cpuWriteInterrupt(u16 ADDRESS, u8 VALUE);

#define ADDR_IF (0xFF0Fu)
#define ADDR_IE (0xFFFFu)
