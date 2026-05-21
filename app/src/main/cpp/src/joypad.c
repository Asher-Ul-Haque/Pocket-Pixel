#include <joypad.h>
#include <cpu/interrupts.h>

static JoypadContext ctx;

JoypadContext* joypadGetContext(void)
{ return &ctx; }

void joypadInit(void)
{
  memset(&ctx, 0, sizeof(ctx));
  ctx.joypSelect = JOYP_SELECT_DPAD_MASK | JOYP_SELECT_BUTTONS_MASK;
}

static u8 buildLowerNibble(void)
{
  u8 lower = JOYP_INPUT_BITS_MASK;

  if ((ctx.joypSelect & JOYP_SELECT_DPAD_MASK) == 0)
  {
    if (ctx.buttons[JOYPAD_BUTTON_RIGHT]) lower &= (u8)~(1u << 0);
    if (ctx.buttons[JOYPAD_BUTTON_LEFT])  lower &= (u8)~(1u << 1);
    if (ctx.buttons[JOYPAD_BUTTON_UP])    lower &= (u8)~(1u << 2);
    if (ctx.buttons[JOYPAD_BUTTON_DOWN])  lower &= (u8)~(1u << 3);
  }

  if ((ctx.joypSelect & JOYP_SELECT_BUTTONS_MASK) == 0)
  {
    if (ctx.buttons[JOYPAD_BUTTON_A])      lower &= (u8)~(1u << 0);
    if (ctx.buttons[JOYPAD_BUTTON_B])      lower &= (u8)~(1u << 1);
    if (ctx.buttons[JOYPAD_BUTTON_SELECT]) lower &= (u8)~(1u << 2);
    if (ctx.buttons[JOYPAD_BUTTON_START])  lower &= (u8)~(1u << 3);
  }

  return lower;
}

u8 joypadRead(void)
{
  return (u8)(JOYP_UNUSED_BITS_MASK | (ctx.joypSelect & (JOYP_SELECT_DPAD_MASK | JOYP_SELECT_BUTTONS_MASK)) | buildLowerNibble());
}

void joypadWrite(u8 VALUE)
{
  ctx.joypSelect &= (u8)~(JOYP_SELECT_DPAD_MASK | JOYP_SELECT_BUTTONS_MASK);
  ctx.joypSelect |= (VALUE & (JOYP_SELECT_DPAD_MASK | JOYP_SELECT_BUTTONS_MASK));
}

void joypadSetButton(JoypadButton BUTTON, bool PRESSED)
{
  if (BUTTON >= JOYPAD_BUTTON_COUNT) return;

  const bool wasPressed = ctx.buttons[BUTTON];
  ctx.buttons[BUTTON]   = PRESSED;

  if (!wasPressed && PRESSED)
  {
    cpuRequestInterrupt(CPU_INT_JOYPAD);
  }
}
