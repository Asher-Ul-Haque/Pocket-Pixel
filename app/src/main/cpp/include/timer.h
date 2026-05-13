#pragma once 
#include <common.h>

/**
 * @file timer.h
 * @brief Timer consists of four registers, which is a 16 bit counter, but only the top 8 bits are visible to the CPU
 * * Registers:
 * - 0xFF04: DIV  (Divider Register)
 * - 0xFF05: TIMA (Timer Counter)
 * - 0xFF06: TMA  (Timer Modulo)
 * - 0xFF07: TAC  (Timer Control)
*/

typedef struct TimerContext
{
  u16 internalCounter;   /// 16 bit counter
  u8  tima;              /// Timer Counter (0xFF05)                       
  u8  tma;               /// Timer Modulo (0xFF06)                    
  u8  tac;               /// Timer Control (0xFF07) 
  u8  timaReloadDelay;   /// T-cycles until TIMA reload after overflow
  bool timaReloadPending;
} TimerContext;

TimerContext* timerGetContext(void);

void timerInit(void);
void timerStepMCycle(void);

u8   timerRead(u16 ADDRESS);
void timerWrite(u16 ADDRESS, u8 VALUE);

#define DIV_REGISTER_ADDRESS  0xFF04
#define TIMA_REGISTER_ADDRESS 0xFF05
#define TMA_REGISTER_ADDRESS  0xFF06
#define TAC_REGISTER_ADDRESS  0xFF07
