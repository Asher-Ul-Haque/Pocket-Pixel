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

// Simple grayscale palette (ARGB)
constexpr u32 TEST_PALETTE[4] = {
        0xFF000000, // Black
        0xFF555555, // Dark Gray
        0xFFAAAAAA, // Light Gray
        0xFFFFFFFF  // White
};

// Generates a checkerboard test pattern with animated motion
void getFrame(u32* buffer) {
    frameCount++;

    for (u64 i = 0; i < FRAME_WIDTH * FRAME_HEIGHT; i++) {
        u64 row = i / FRAME_WIDTH;
        u64 col = i % FRAME_WIDTH;

        // Checker pattern that shifts every ~10 frames
        u8 colorIndex = ((row / 8 + col / 8 + frameCount / 10) % 4);

        buffer[i] = TEST_PALETTE[colorIndex];
    }
}

// Dummy audio output
void getAudio(u8* buffer) {
    for (u64 i = 0; i < FRAME_WIDTH; ++i) {
        buffer[i] = 0b11001100; // Dummy tone pattern
    }
}

// Button input handling (optional logging)
void setButton(u8 button, bool pressed) {
    FORGE_LOG_INFO("Button %d (pressed=%d)", button, pressed);
}
