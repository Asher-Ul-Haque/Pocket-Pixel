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

// --- Timing, FPS, & Auto-Save Variables ---
static u64 lastFrameTime = 0;
static double timeAccumulator = 0.0;
static u32 framesThisSecond = 0;
static u64 lastFpsTime = 0;
static u32 autoSaveFrameCounter = 0; // Synchronizes game states to IndexedDB

// ============================================================================
// --- NATIVE FILE SYSTEM PERSISTENCE CORE ---
// ============================================================================

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) {
    if (!RAM_DATA || RAM_SIZE == 0) return false;
    
    // Wrap raw WebAssembly pointer into a shared JS ArrayBuffer view on the fly
    EM_ASM({
        if (window.PocketEngine && window.PocketEngine.onSaveRamFlush) {
            var heapBytes = new Uint8Array(Module.HEAPU8.buffer, $0, $1);
            var persistentCopy = new Uint8Array(heapBytes); // Hard detached clone
            window.PocketEngine.onSaveRamFlush(persistentCopy.buffer);
        }
    }, RAM_DATA, RAM_SIZE);

    return true;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) {
    if (!RAM_DATA || RAM_SIZE == 0) return false;

    // Pull directly out of pre-staged JS memory array synchronously
    int success = EM_ASM_INT({
        if (window.currentSaveBuffer) {
            var src = new Uint8Array(window.currentSaveBuffer);
            var copySize = Math.min($1, src.length);
            var dest = new Uint8Array(Module.HEAPU8.buffer, $0, copySize);
            
            dest.set(src.subarray(0, copySize));
            return 1;
        }
        return 0;
    }, RAM_DATA, RAM_SIZE);

    return (success == 1);
}

u32 fileGetExpectedSaveSize(void) {
    CartContext* ctx = cartridgeGetContext();
    if (!ctx || !ctx->initialized) return 0;
    
    // Automatically scale memory allocation arrays when live timing hardware is reported
    return ctx->externalRamSize + (ctx->hasRTC ? 64 : 0);
}

// ============================================================================
// --- MAIN EXECUTION ENGINE ---
// ============================================================================

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

        // --- BACKGROUND DAEMON IMMUNIZATION CYCLE ---
        // Once every 300 frames (~5 seconds), flush modifications to database safely.
        // It immediately exits if the dirty layout parameters remain unaltered.
        autoSaveFrameCounter++;
        if (autoSaveFrameCounter >= 300) {
            autoSaveFrameCounter = 0;
            cartridgeFlushRAM();
        }
    }

    // 4. Print the FPS Counter to the Terminal every 1000ms
    if (currentTime - lastFpsTime >= 1000) {
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

// ==========================================
// --- SAVE STATE BRIDGE ---
// ==========================================

EMSCRIPTEN_KEEPALIVE
void* webAllocate(u32 size) {
    return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void webFree(void* ptr) {
    if (ptr) free(ptr);
}

// --- Cache lines to track active DMG palette configuration for screenshots ---
static u32 current_c0 = 0xD1CB95FF;
static u32 current_c1 = 0x40985EFF;
static u32 current_c2 = 0x1A644EFF;
static u32 current_c3 = 0x04373BFF;

// FIXED: Receives primitive hex formats via standard number signatures
EMSCRIPTEN_KEEPALIVE
void webSetPalette(u32 c0, u32 c1, u32 c2, u32 c3) {
    current_c0 = c0;
    current_c1 = c1;
    current_c2 = c2;
    current_c3 = c3;
    if (platform && platform->video.setDmgPalette) {
        DmgPalette p = {c0, c1, c2, c3};
        platform->video.setDmgPalette(p);
    }
}

EMSCRIPTEN_KEEPALIVE
void webSetPaused(i32 paused) {
    if (platform) platform->input.paused = (paused != 0);
}

EMSCRIPTEN_KEEPALIVE
i32 webIsPaused(void) {
    if (platform) return platform->input.paused ? 1 : 0;
    return 0;
}

EMSCRIPTEN_KEEPALIVE
u32* webCaptureFrameBuffer(void) {
    if (!ppu) return NULL;

    u32* buffer = (u32*)malloc(160 * 144 * sizeof(u32));
    if (!buffer) return NULL;

    GameBoyMode romMode = cartridgeGetContext()->mode;
    bool isCgbMode = (romMode == MODE_CGB_GAMEBOY || romMode == MODE_CGB_ONLY_GAMEBOY);

    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            int index = (y * 160) + x;
            u16 rawCoreData = ppu->frameBuffer.pixels[y][x];

            if (!isCgbMode) {
                switch (rawCoreData) {
                    case 0: buffer[index] = current_c0; break;
                    case 1: buffer[index] = current_c1; break;
                    case 2: buffer[index] = current_c2; break;
                    case 3: buffer[index] = current_c3; break;
                    default: buffer[index] = 0xFF00FFFF; break;
                }
            } else {
                u8 r5 = (rawCoreData & 0x001F);
                u8 g5 = (rawCoreData & 0x03E0) >> 5;
                u8 b5 = (rawCoreData & 0x7C00) >> 10;

                u8 r8 = (r5 << 3) | (r5 >> 2);
                u8 g8 = (g5 << 3) | (g5 >> 2);
                u8 b8 = (b5 << 3) | (b5 >> 2);

                buffer[index] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
            }
        }
    }
    return buffer;
}
