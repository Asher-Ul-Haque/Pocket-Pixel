#include "GameBoyCore.h"
#include "ForgeLib/include/asserts.h"
#include "ForgeLib/include/logger.h"

static bool started = false;
static u64 frameCount = 0;
#define FRAME_WIDTH 160

void startEmulator()
{
    FORGE_ASSERT_MESSAGE(!started, "Cannot start the emulator when it is already running");
    started = true;
    FORGE_LOG_INFO("Started the emulator");
}

void stopEmulator()
{
    FORGE_ASSERT_MESSAGE(started, "Cannot stop the emulator when it is not running");
    started = false;
    FORGE_LOG_INFO("Stopped the emulator");
}

void stepFrame()
{
    frameCount++;
}

void getFrame(u8* BUFFER)
{
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

void loadROM(u8* ROM){}

void setButton(u8 BUTTON, bool PRESSED)
{
    FORGE_LOG_INFO("Recieved Button %d", BUTTON)
}