#pragma once

#include <common.h>

#define JOYP_REGISTER_ADDRESS 0xFF00

#define JOYP_SELECT_DPAD_MASK     0x10
#define JOYP_SELECT_BUTTONS_MASK  0x20
#define JOYP_UNUSED_BITS_MASK     0xC0
#define JOYP_INPUT_BITS_MASK      0x0F

typedef enum JoypadButton
{
  JOYPAD_BUTTON_RIGHT,
  JOYPAD_BUTTON_LEFT,
  JOYPAD_BUTTON_UP,
  JOYPAD_BUTTON_DOWN,
  JOYPAD_BUTTON_A,
  JOYPAD_BUTTON_B,
  JOYPAD_BUTTON_SELECT,
  JOYPAD_BUTTON_START,
  JOYPAD_BUTTON_COUNT
} JoypadButton;

typedef struct JoypadContext
{
  u8   joypSelect;
  bool buttons[JOYPAD_BUTTON_COUNT];
} JoypadContext;

JoypadContext* joypadGetContext(void);

void joypadInit(void);
u8   joypadRead(void);
void joypadWrite(u8 VALUE);
void joypadSetButton(JoypadButton BUTTON, bool PRESSED);
