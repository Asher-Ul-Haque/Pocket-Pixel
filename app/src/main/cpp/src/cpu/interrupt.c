/**
 * @file interrupt.c
 * @brief Interrupt pending detection and interrupt entry microsequence.
 *
 * This is intentionally separated so:
 * - HALT behavior can query "any pending" without embedding IO details in ops files
 * - interrupt entry sequence can be cycle-stepped cleanly
 *
 * IMPORTANT:
 * This file needs access to IE (0xFFFF) and IF (0xFF0F).
 * For now, we read/write them via busRead/busWrite. Later you can route through IO helpers.
*/

#include <cpu/interrupts.h>
#include <cpu/cpu.h>
#include <bus.h>


static InterruptContext ctx;
InterruptContext* cpuInterruptGetContext(void)
{ return &ctx; }

bool cpuInterruptPending(void)
{
  return (u8)(ctx.interruptEnable & ctx.interruptFlag & 0x1Fu) != 0;
}

CpuInterrupt cpuInterruptGetHighest(void)
{
  const u8 pending = (u8)(ctx.interruptEnable & ctx.interruptFlag & 0x1Fu);

  for (i8 bit = 0; bit < 5; ++bit)
  {
    if (pending & (1 << bit)) return (CpuInterrupt)bit;
  }

  return CPUT_INT_NONE;
}

void cpuInterruptAcknowledge(CpuInterrupt INT)
{
  ctx.interruptFlag &= (u8)~(1 << (u8) INT);
  ctx.interruptFlag &= 0x1Fu;
}

u16 cpuInterruptVector(CpuInterrupt INT)
{
  switch (INT)
  {
    case CPU_INT_VBLANK : return CPU_INT_VEC_VBLANK;
    case CPU_INT_LCD    : return CPU_INT_VEC_LCD;
    case CPU_INT_TIMER  : return CPU_INT_VEC_TIMER;
    case CPU_INT_SERIAL : return CPU_INT_VEC_SERIAL;
    case CPU_INT_JOYPAD : return CPU_INT_VEC_JOYPAD;
    default: return 0;
  }
}

void cpuRequestInterrupt(CpuInterrupt INT)
{
  ctx.interruptFlag |= (u8)(1 << (u8)INT);
  ctx.interruptFlag &= 0x1Fu;
}

u8 cpuReadInterrupt(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    case ADDR_IF: return ctx.interruptFlag | 0xE0; /// Top 3 bits are unused
    case ADDR_IE: return ctx.interruptEnable | 0xE0;
    default:
    FORGE_LOG_ERROR("Attempted to read from invalid interrupt address: 0x%04X", ADDRESS);
          FORGE_ASSERT_DEBUG(false, "Invalid interrupt read address");
  }
}

void cpuWriteInterrupt(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    case ADDR_IF: ctx.interruptFlag   = (u8)(VALUE & 0x1Fu); break;
    case ADDR_IE: ctx.interruptEnable = (u8)(VALUE & 0x1Fu); break;
    default:
    FORGE_LOG_ERROR("Attempted to write to invalid interrupt address: 0x%04X", ADDRESS);
          FORGE_ASSERT_DEBUG(false, "Invalid interrupt write address");
  }
}
