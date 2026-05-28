#include <bus.h>
#include <time.h>
#include <utils/asserts.h>
#include <cartridge/memoryBankController.h>
#include <cartridge/cartridge.h>
#include <common.h>

static inline u32 mbc3GetRomBankCount(const CartContext* CTX)
{
  u32 count = CTX->romSize / ROM_BANK_SIZE;
  return (count == 0) ? 1 : count;
}

static inline u32 mbc3GetRamBankCount(const CartContext* CTX)
{
  u32 count = CTX->externalRamSize / RAM_BANK_SIZE;
  return count; // - - -  can be 0
}

static inline u8 mbc3NormalizeRomBank(u8 BANK_7, const CartContext* CTX)
{
  u8 b = (u8)(BANK_7 & MBC3_ROM_BANK_MASK);
  if (b == 0) b = 1;

  // - - - clamp into range
  b %= (u8)mbc3GetRomBankCount(CTX);
  if (b == 0) b = 1;
  return b;
}

static inline bool mbc3IsRtcSelected(u8 SEL)
{
  return (SEL >= MBC3_RTC_SECONDS_REGISTER && SEL <= MBC3_RTC_DAY_COUNTER_HI);
}

/**
 * Advance the live RTC by delta seconds (MBC3 style).
 * - seconds 0-59, minutes 0-59, hours 0-23, days 0-511
 * - dayCarry set when overflow beyond 511
*/
static void mbc3RtcAdvanceSeconds(MBC3State* STATE, u32 DELTA_SECONDS)
{
  if (DELTA_SECONDS == 0) return;
  if (STATE->rtcHalt) return;

  u32 sec           = (u32)STATE->rtcSeconds + DELTA_SECONDS;
  u32 carryMin      = sec / 60u;
  STATE->rtcSeconds = (u8)(sec % 60u);

  if (carryMin)
  {
    u32 min             = (u32)STATE->rtcMinutes + carryMin;
    u32 carryHour       = min / 60u;
    STATE->rtcMinutes   = (u8)(min % 60u);

    if (carryHour)
    {
      u32 hour        = (u32)STATE->rtcHours + carryHour;
      u32 carryDay    = hour / 24u;
      STATE->rtcHours = (u8)(hour % 24u);

      if (carryDay)
      {
        u32 days = (u32)STATE->rtcDays + carryDay;
        if (days >= 512u)
        {
          // - - -  overflow/carry
          STATE->rtcDayCarry = true;
          days %= 512u;
        }
        STATE->rtcDays = (u16)days;
      }
    }
  }
}

static void mbc3RtcUpdateNow(CartContext* CTX)
{
  if (!CTX->hasRTC) return;

  MBC3State* state = &CTX->mapper.mbc3;

  time_t now = time(NULL);
  if (state->lastSystemTime == 0)
  {
    state->lastSystemTime = now;
    return;
  }

  if (now <= state->lastSystemTime) return;

  // - - - If halted, do NOT advance the clock, but do update the last system time so that we can resume from this point when un-halted.
  if (state->rtcHalt)
  {
    state->lastSystemTime = now;
    return;
  }

  u32 delta             = (u32)(now - state->lastSystemTime);
  state->lastSystemTime = now;

  mbc3RtcAdvanceSeconds(state, delta);
}

static void mbc3RtcLatch(CartContext* CTX)
{
  MBC3State* s = &CTX->mapper.mbc3;

  // - - - Latch captures the *current live* values (after updating to now).
  mbc3RtcUpdateNow(CTX);

  s->latchedSeconds   = s->rtcSeconds;
  s->latchedMinutes   = s->rtcMinutes;
  s->latchedHours     = s->rtcHours;
  s->latchedDays      = s->rtcDays;
  s->latchedHalt      = s->rtcHalt;
  s->latchedDayCarry  = s->rtcDayCarry;

  s->latched = true;
}

/**
 * Read RTC register (latched if latched==true, otherwise typical emus return live
 * but hardware tends to require latch for stable reads; we enforce latch for safety.)
*/
static u8 mbc3RtcReadReg(const CartContext* CTX, u8 REGISTER)
{
  CartContext* ctx = (CartContext*)cartridgeGetContext();
  mbc3RtcUpdateNow(ctx);
  const MBC3State* s = &CTX->mapper.mbc3;

  const bool useLatched = s->latched;
  const u8   seconds    = useLatched ? s->latchedSeconds    : s->rtcSeconds;
  const u8   minutes    = useLatched ? s->latchedMinutes    : s->rtcMinutes;
  const u8   hours      = useLatched ? s->latchedHours      : s->rtcHours;
  const u16  days       = useLatched ? s->latchedDays       : s->rtcDays;
  const bool halt       = useLatched ? s->latchedHalt       : s->rtcHalt;
  const bool carry      = useLatched ? s->latchedDayCarry   : s->rtcDayCarry;

  switch (REGISTER)
  {
    case MBC3_RTC_SECONDS_REGISTER : return (u8)(seconds % 60u);
    case MBC3_RTC_MINUTES_REGISTER : return (u8)(minutes % 60u);
    case MBC3_RTC_HOURS_REGISTER   : return (u8)(hours % 24u);

    case MBC3_RTC_DAY_COUNTER_LOW  : return (u8)(days & 0xFFu);

    case MBC3_RTC_DAY_COUNTER_HI:
    {
      u8 dayHi  = (u8)((days >> 8) & 0x01u);
      u8 v      = 0;

      if (dayHi)   v |= MBC3_RTC_DH_DAY_HI_BIT;
      if (halt)    v |= MBC3_RTC_DH_DAY_HALT_BIT;
      if (carry)   v |= MBC3_RTC_DH_DAY_CARRY_BIT;
      return v;
    }

    default: return OPEN_BUS_VALUE;
  }
}

static void mbc3RtcWriteReg(CartContext* CTX, u8 REGISTER, u8 VALUE)
{
  MBC3State* s = &CTX->mapper.mbc3;

  mbc3RtcUpdateNow(CTX);

  switch (REGISTER)
  {
    case MBC3_RTC_SECONDS_REGISTER : s->rtcSeconds = (u8)(VALUE % 60u); break;
    case MBC3_RTC_MINUTES_REGISTER : s->rtcMinutes = (u8)(VALUE % 60u); break;
    case MBC3_RTC_HOURS_REGISTER   : s->rtcHours   = (u8)(VALUE % 24u); break;

    case MBC3_RTC_DAY_COUNTER_LOW:
      s->rtcDays = (u16)((s->rtcDays & 0x0100u) | (u16)VALUE);
      break;

    case MBC3_RTC_DAY_COUNTER_HI:
    {
      u16 dayHi  = (u16)(VALUE & MBC3_RTC_DH_DAY_HI_BIT);
      s->rtcDays = (u16)((s->rtcDays & 0x00FFu) | (dayHi << 8));
      s->rtcHalt = (VALUE & MBC3_RTC_DH_DAY_HALT_BIT) ? true : false;

      // - - - Writing carry bit is allowed on many implementatcartridgens; treat it as set/clear.
      s->rtcDayCarry = (VALUE & MBC3_RTC_DH_DAY_CARRY_BIT) ? true : false;
      break;
    }

    default: break;
  }
}


// - - - Read / Write - - - 

u8 mbc3Read(u16 ADDRESS)
{
  const CartContext* ctx = cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC3] : Cartridge not initialized");

  // - - - ROM reads
  if (ADDRESS <= ADDR_ROMX_END)
  {
    if (ADDRESS <= ADDR_ROM0_END)
    {
      // - - - Bank 0 fixed for MBC3 
      if ((u32)ADDRESS < ctx->romSize) return ctx->romData[ADDRESS];
      return OPEN_BUS_VALUE;
    }

    // - - - Banked regcartridgen 4000-7FFF
    u8  bank        = mbc3NormalizeRomBank(ctx->mapper.mbc3.romBank7, ctx);
    u32 addrInBank  = (u32)(ADDRESS - ADDR_ROMX_START);
    u32 romIndex    = (u32)bank * ROM_BANK_SIZE + addrInBank;

    if (romIndex < ctx->romSize) return ctx->romData[romIndex];
    return OPEN_BUS_VALUE;
  }

  // - - - External RAM / RTC
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->ramEnabled) return OPEN_BUS_VALUE;

    u8 sel = ctx->mapper.mbc3.ramBankOrRtcReg;

    // - - - RTC regs
    if (mbc3IsRtcSelected(sel))
    {
      if (!ctx->hasRTC) return OPEN_BUS_VALUE;
      return mbc3RtcReadReg(ctx, sel);
    }

    // - - - RAM banks 0-3
    if (!ctx->hasRam || !ctx->externalRamData || ctx->externalRamSize == 0) return OPEN_BUS_VALUE;

    u8  ramBank  = (u8)(sel & 0x03u);
    u32 ramBanks = mbc3GetRamBankCount(ctx);
    if (ramBanks == 0) return OPEN_BUS_VALUE;
    ramBank = (u8)(ramBank % ramBanks);

    u32 offset   = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)ramBank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize) return ctx->externalRamData[ramIndex];
    return OPEN_BUS_VALUE;
  }

  return OPEN_BUS_VALUE;
}

void mbc3Write(u16 ADDRESS, u8 VALUE)
{
  CartContext* ctx = (CartContext*)cartridgeGetContext();
  FORGE_ASSERT_MESSAGE(ctx->initialized, "[MBC3] : Cartridge not initialized");

  // - - - 0000-1FFF: RAM/RTC enable
  if (ADDRESS <= ADDR_RAM_ENABLE_END)
  {
    ctx->ramEnabled = ((VALUE & MBC_RAM_ENABLE_MASK) == MBC_RAM_ENABLE_VALUE);
    return;
  }

  // - - - 2000-3FFF: ROM bank number (7 bits, 0->1)
  if (ADDRESS <= ADDR_ROM_BANK_END)
  {
    ctx->mapper.mbc3.romBank7 = (u8)(VALUE & MBC3_ROM_BANK_MASK);
    if ((ctx->mapper.mbc3.romBank7 & MBC3_ROM_BANK_MASK) == 0)
    {
      ctx->mapper.mbc3.romBank7 = 1;
    }

    ctx->romBank = mbc3NormalizeRomBank(ctx->mapper.mbc3.romBank7, ctx);
    return;
  }

  // - - - 4000-5FFF: RAM bank number (0-3) OR RTC register select (0x08-0x0C)
  if (ADDRESS <= ADDR_RAM_RTC_SEL_END)
  {
    ctx->mapper.mbc3.ramBankOrRtcReg = VALUE;
    if (!mbc3IsRtcSelected(VALUE))  ctx->ramBank = (u8)(VALUE & 0x03u);
    return;
  }

  // - - - 6000-7FFF: Latch clock data (0->1 transitcartridgen)
  if (ADDRESS <= ADDR_LATCH_END)
  {
    if (!ctx->hasRTC) 
    {
      ctx->mapper.mbc3.latchPrev = VALUE;
      return;
    }

    u8 prev = ctx->mapper.mbc3.latchPrev;
    ctx->mapper.mbc3.latchPrev = VALUE;

    // - - - latch on 0 -> 1 transitcartridgen
    if (prev == 0x00 && VALUE == 0x01)
    {
      mbc3RtcLatch(ctx);
    }
    return;
  }

  // - - - A000-BFFF: RAM or RTC write
  if (ADDRESS >= ADDR_RAM_START && ADDRESS <= ADDR_RAM_END)
  {
    if (!ctx->ramEnabled) return;

    u8 sel = ctx->mapper.mbc3.ramBankOrRtcReg;

    if (mbc3IsRtcSelected(sel))
    {
      if (!ctx->hasRTC) return;
      mbc3RtcWriteReg(ctx, sel, VALUE);
      return;
    }

    // - - - RAM bank write 
    if (!ctx->hasRam || !ctx->externalRamData || ctx->externalRamSize == 0) return;

    u8  ramBank  = (u8)(sel & 0x03u);
    u32 ramBanks = mbc3GetRamBankCount(ctx);
    if (ramBanks == 0) return;
    ramBank = (u8)(ramBank % ramBanks);

    u32 offset   = (u32)(ADDRESS - ADDR_RAM_START);
    u32 ramIndex = (u32)ramBank * RAM_BANK_SIZE + offset;

    if (ramIndex < ctx->externalRamSize)
    {
      ctx->externalRamData[ramIndex] = VALUE;
      ctx->ramDirty = true;
    }
    return;
  }
}
