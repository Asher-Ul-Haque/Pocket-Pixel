/**
 * @file joypad.h 
 * @brief Emulates the gmae boy joypad 
*/

#pragma once
#include <common.h>

// - - - Magic values - - - 

#define JOYP_REGISTER_ADDRESS     0xFF00
#define JOYP_SELECT_DPAD_MASK     0x10
#define JOYP_SELECT_BUTTONS_MASK  0x20
#define JOYP_UNUSED_BITS_MASK     0xC0
#define JOYP_INPUT_BITS_MASK      0x0F


/// @brief Game boy joypad buttons
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

/// @brief Joypad context 
typedef struct JoypadContext
{
  u8   joypSelect;
  bool buttons[JOYPAD_BUTTON_COUNT];
} JoypadContext;

/**
 * @brief Global access to the joypad context 
 * @return A pointer to the global context for the joypad 
 * @note This will never return a null pointer
*/
JoypadContext* joypadGetContext(void);

/// @brief initialize the joypad 
void joypadInit(void);

/**
 * @brief Returns a byte from the joypad registers 
 * @return a byte from the joypad buttons
*/
u8   joypadRead(void);

/**
 * @brief Writes a byte from the joypad registers 
 * @param VALUE what to write
*/
void joypadWrite(u8 VALUE);

/**
 * @brief Set a button on the joypad 
 * @param BUTTON which button to press 
 * @param PRESSED whether to press or release 
*/
void joypadSetButton(JoypadButton BUTTON, bool PRESSED);
