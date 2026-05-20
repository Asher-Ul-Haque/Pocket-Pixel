#pragma once

#include <common.h>

#define JOYP_REGISTER_ADDRESS 0xFF00

#define JOYP_SELECT_DPAD_MASK     0x10
#define JOYP_SELECT_BUTTONS_MASK  0x20
#define JOYP_UNUSED_BITS_MASK     0xC0
#define JOYP_INPUT_BITS_MASK      0x0F

typedef enum JoypadButton
{
  JOYPAD_BUTTON_RIGHT = 0,
  JOYPAD_BUTTON_LEFT  = 1,
  JOYPAD_BUTTON_UP    = 2,
  JOYPAD_BUTTON_DOWN  = 3,
  JOYPAD_BUTTON_A     = 4,
  JOYPAD_BUTTON_B     = 5,
  JOYPAD_BUTTON_SELECT= 6,
  JOYPAD_BUTTON_START = 7,
  JOYPAD_BUTTON_COUNT = 8
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
