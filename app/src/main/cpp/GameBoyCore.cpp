#include "GameBoyCore.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoy/include/apu.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/ppu.h"
#include "GameBoy/include/timer.h"
#include "GameBoy/include/emu.h"
#include "GameBoy/include/gamepad.h"

u64 frameCount = 0;

void stopEmulator() 
{
  FORGE_LOG_INFO("Emulator stopped");
  emuGetContext()->running = false;
  if (ppuGetContext()->frameBuffer) free(ppuGetContext()->frameBuffer);
}

void startEmulator()
{
  FORGE_LOG_INFO("Emulator started");
  emuGetContext()->running = true;
  emuGetContext()->ticks = 0;
  timerInit();
  cpuInit();
  ppuInit();
  apuInit();
}

void setButton(Buttons BUTTON, bool PRESSED) 
{
  GamepadState* state = gamepadGetState();
  switch (BUTTON)
  {
    case UP    : { state->up    = PRESSED; break; }
    case DOWN  : { state->down  = PRESSED; break; }
    case LEFT  : { state->left  = PRESSED; break; }
    case RIGHT : { state->right = PRESSED; break; }

    case A : { state->a = PRESSED; break; }
    case B : { state->b = PRESSED; break; }

    case START  : { state->start  = PRESSED; break; } 
    case SELECT : { state->select = PRESSED; break; }
  }
}
