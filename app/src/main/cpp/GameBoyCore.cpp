#include "GameBoyCore.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "Machine/Parts/cpu.h"

static u64 frameCount = 0;
#define FRAME_WIDTH 160


void stopEmulator()
{ FORGE_LOG_INFO("Emulator Stopped");}

void startEmulator()
{
  FORGE_LOG_INFO("Emulator Start");
  cpuInit();
}

void getFrame(u8* BUFFER)
{
  frameCount++;
  u64 width  = 160;
  u64 height = 144;

  for (u64 i = 0; i < width * height; i += 4)
  {
    u64 row = (i / width);
    u64 col = (i % width);

    u8 color1 = ((row / 8 + col / 8 + frameCount / 10) % 2) * 3; // 2-bit grayscale
    u8 color2 = ((row / 8 + (col + 1) / 8 + frameCount / 10) % 2) * 3;
    u8 color3 = ((row / 8 + (col + 2) / 8 + frameCount / 10) % 2) * 3;
    u8 color4 = ((row / 8 + (col + 3) / 8 + frameCount / 10) % 2) * 3;

    BUFFER[i / 4] = (color1 << 6) | (color2 << 4) | (color3 << 2) | (color4);
  }
}

void getAudio(u8* BUFFER)
{
  for (u64 i = 0; i < FRAME_WIDTH; ++i)
  {
     BUFFER[i] = 0b11001100; // - - - temp
  }
}

void setButton(u8 BUTTON, bool PRESSED)
{
  FORGE_LOG_INFO("Received Button %d", BUTTON)
}

void cycles(u32 CPU_CYCLES)
{ }
