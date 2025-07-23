#include "../include/gamepad.h"
#include <string.h>

typedef struct 
{
  bool          buttonSel;
  bool          dirSel;
  GamepadState  controller;
} GamepadContext;

static GamepadContext ctx =  {0};

bool gamepadButtonSel()
{ return ctx.buttonSel; }

bool gamepadDirSel()
{ return ctx.dirSel; }

void gamepadWrite(u8 VALUE)
{
  ctx.buttonSel = VALUE & 0x20;
  ctx.dirSel    = VALUE & 0x10;
}

GamepadState* gamepadGetState()
{ return &ctx.controller; }

u8 gamepadRead()
{
  u8 output = 0xCF;

  if (!gamepadButtonSel())
  {
    if (!!gamepadGetState()->start)  output &= ~(1 << 3);
    if (!!gamepadGetState()->select) output &= ~(1 << 2);
    if (!!gamepadGetState()->a)      output &= ~(1 << 0);
    if (!!gamepadGetState()->b)      output &= ~(1 << 1);
  }

  if (!gamepadDirSel())
  {
    if (!!gamepadGetState()->left)  output &= ~(1 << 1);
    if (!!gamepadGetState()->right) output &= ~(1 << 0);
    if (!!gamepadGetState()->up)    output &= ~(1 << 2);
    if (!!gamepadGetState()->down)  output &= ~(1 << 3);
  }

  return output;
}
