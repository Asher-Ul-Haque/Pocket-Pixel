#include "GameBoyCore.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/ppu.h"
#include "GameBoy/include/timer.h"
#include "GameBoy/include/emu.h"

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
  u32* framebuffer = (u32*) malloc(sizeof(u32) * 160 * 144);
  ppuInit(framebuffer);
}

// Simple grayscale palette (ARGB)
constexpr u32 TEST_PALETTE[4] = 
  {
    0xFF000000, // Black
    0xFF555555, // Dark Gray
    0xFFAAAAAA, // Light Gray
    0xFFFFFFFF  // White
  };


// Dummy audio output
void getAudio(u8* buffer) 
{

}

// Button input handling (optional logging)
void setButton(u8 button, bool pressed) 
{
  FORGE_LOG_INFO("Button %d (pressed=%d)", button, pressed);
}
