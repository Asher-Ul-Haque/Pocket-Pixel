#include "GameBoyCore.h"
#include "ForgeLibrary/include/asserts.h"
#include "ForgeLibrary/include/logger.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/timer.h"
#include "GameBoy/include/emu.h"

static u64 frameCount = 0;
#define FRAME_WIDTH 160
#define FRAME_HEIGHT 144

void stopEmulator() {
    FORGE_LOG_INFO("Emulator stopped");
    emuGetContext()->running = false;
}

void startEmulator() {
    FORGE_LOG_INFO("Emulator started");
    emuGetContext()->running = true;
    emuGetContext()->ticks = 0;
    timerInit();
    cpuInit();
}

// DMG palette (ARGB)
constexpr u32 DMG_PALETTE[4] = {
        0xFF9BBC0F, // Lightest
        0xFF8BAC0F, // Light
        0xFF306230, // Dark
        0xFF0F380F  // Darkest
};

void getFrame(u16* buffer) {
    frameCount++;
    for (u64 i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++) {
        u64 row = i / FRAME_WIDTH;
        u64 col = i % FRAME_WIDTH;

        // Checkerboard pattern
        u8 colorIndex = ((row / 8 + col / 8 + frameCount / 10) % 2) * 3;

        // Game Boy greens
        u32 colors[4] = {
                0x4208, // DarkGreen   (0xff0f380f) → RGB565
                0x73AE, // MediumGreen (0xff306230)
                0xBDF0, // Green       (0xff8bac0f)
                0xC63F  // LightGreen  (0xff9bbc0f)
        };

        buffer[i] = colors[colorIndex];
    }
}


void getAudio(u8* buffer) {
    for (u64 i = 0; i < FRAME_WIDTH; ++i) {
        buffer[i] = 0b11001100; // Dummy tone
    }
}

void setButton(u8 button, bool pressed) {
    FORGE_LOG_INFO("Received button %d (pressed=%d)", button, pressed);
}
