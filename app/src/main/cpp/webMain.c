#include <apu/apu.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <emscripten.h>

#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <debug.h>
#include <joypad.h>
#include "state.h" // Included for memory-based save states

static bool running              = true;
static PlatformContext* platform = NULL;
static PpuContext* ppu           = NULL;

// --- Timing & FPS Variables ---
static u64 lastFrameTime = 0;
static double timeAccumulator = 0.0;
static u32 framesThisSecond = 0;
static u64 lastFpsTime = 0;

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) { (void)RAM_DATA; (void)RAM_SIZE; return true; }
bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) { (void)RAM_DATA; (void)RAM_SIZE; return false; }
u32 fileGetExpectedSaveSize(void) { return 0; }

void emscripten_main_loop(void) {
    // 1. Calculate precise Delta Time
    u64 currentTime = SDL_GetTicks();
    if (lastFrameTime == 0) {
        lastFrameTime = currentTime;
        lastFpsTime = currentTime;
    }
    
    double deltaTime = (double)(currentTime - lastFrameTime);
    lastFrameTime = currentTime;

    // Prevent the "spiral of death" if the user switches browser tabs
    if (deltaTime > 100.0) deltaTime = 100.0;
    
    timeAccumulator += deltaTime;

    // A Game Boy runs at exactly 59.7275 Hz. (1000ms / 59.7275 = ~16.742ms per frame)
    double targetFrameTimeMs = 16.742;
    if (platform && platform->input.doubleSpeed) {
        targetFrameTimeMs /= 2.0; 
    }

    bool frameRendered = false;

    // 2. Consume accumulated time in mathematically perfect Game Boy frames
    while (timeAccumulator >= targetFrameTimeMs) {
        
        if (platform && platform->input.poll) {
            platform->input.poll(&running);
        }

        if (!running) {
            emscripten_cancel_main_loop();
            return;
        }

        // --- Hardware Loop ---
        if (platform && !platform->input.paused) {
            int mCyclesThisFrame = 0;
            const int MAX_MCYCLES = 36000; // LCD-Off Breaker

            while (running && !ppu->frameReady && mCyclesThisFrame < MAX_MCYCLES) {
                cpuTick();
                if (cpuGetContext()->stopped) {
                    if (ppu->registers.key1 & 0x01) {
                        ppuExecuteSpeedSwitch();
                        cpuGetContext()->stopped = false;
                    }
                }
                timerStepMCycle();
                apuTick();

                u8 dotsToTick = (ppu->registers.key1 & 0x80) ? 2 : 4;
                for (u8 i = 0; i < dotsToTick; ++i) {
                    ppuTick();
                }
                mCyclesThisFrame++;
            }
            ppu->frameReady = false;
            frameRendered = true;
        }

        timeAccumulator -= targetFrameTimeMs;
    }

    // 3. Render only if a frame was actually generated
    if (frameRendered && platform) {
        if (platform->video.renderFrame) platform->video.renderFrame(&ppu->frameBuffer);
        if (platform->video.present) platform->video.present();
        
        framesThisSecond++;
    }

    // 4. Print the FPS Counter to the Terminal every 1000ms
    if (currentTime - lastFpsTime >= 1000) {
        char fpsString[64];
        snprintf(fpsString, sizeof(fpsString), "[SYSTEM] Emulation FPS: %u", framesThisSecond);
        FORGE_LOG_INFO("%s", fpsString);
        
        framesThisSecond = 0;
        lastFpsTime = currentTime;
    }
}

i32 main(int ARGUMENT_COUNT, char* ARGUMENT_VECTOR[]) {
    if (ARGUMENT_COUNT < 2) {
        FORGE_LOG_FATAL("%s", "Web Build: Waiting for ROM upload via JS bridge...");
        return 0;
    }

    const char* romPath = ARGUMENT_VECTOR[1];
    FILE* file = fopen(romPath, "rb");
    if (!file) {
        FORGE_LOG_FATAL("Failed to open ROM from virtual FS: %s", romPath);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    u64 romSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    u8* romData = (u8*)malloc(romSize);
    if (!romData || fread(romData, 1, (size_t)romSize, file) != romSize) {
        if (romData) free(romData);
        fclose(file);
        return 1;
    }
    fclose(file);

    CartridgeFileIO fileIO = {
        .saveRamToFile       = fileSaveRam,
        .loadRamFromFile     = fileLoadRam,
        .getExpectedSaveSize = fileGetExpectedSaveSize
    };

    if (!cartridgeInit(&fileIO, romData, (u32)romSize)) {
        FORGE_LOG_FATAL("%s", "Failed to init cartridge on web");
        free(romData);
        return 1;
    }

    platformInit();
    joypadInit();
    cpuInit();
    ppuInit();
    apuInit();
    timerInit();
    serialInit();

    platform = platformGetContext();
    ppu = ppuGetContext();

    FORGE_LOG_INFO("%s", "--- POCKET PIXEL WEB STARTING ---");

    // Pass 0 to sync directly with the monitor's requestAnimationFrame
    emscripten_set_main_loop(emscripten_main_loop, 0, 1);

    return 0;
}


// ============================================================================
// --- JAVASCRIPT EXPORTS (WASM API) ---
// These functions are called directly from your web UI using Emscripten's ccall
// ============================================================================

EMSCRIPTEN_KEEPALIVE
u8* webSaveState(u32* outSize) {
    return systemSaveStateToMemory(outSize);
}

EMSCRIPTEN_KEEPALIVE
bool webLoadState(const u8* buffer, u32 size) {
    return systemLoadStateFromMemory(buffer, size);
}

EMSCRIPTEN_KEEPALIVE
void webFreeStateBuffer(u8* buffer) {
    if (buffer) free(buffer);
}

EMSCRIPTEN_KEEPALIVE
void webSetChannelVolumes(float ch1, float ch2, float ch3, float ch4) {
    apuSetChannelVolumes(ch1, ch2, ch3, ch4);
}

EMSCRIPTEN_KEEPALIVE
void webSetPalette(u32 c0, u32 c1, u32 c2, u32 c3) {
    if (platform && platform->video.setDmgPalette) {
        DmgPalette p = {c0, c1, c2, c3};
        platform->video.setDmgPalette(p);
    }
}
