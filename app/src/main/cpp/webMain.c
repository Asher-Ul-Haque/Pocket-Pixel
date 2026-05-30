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
#include <joypad.h>
#include <state.h> // Included for memory-based save states

static bool running              = true;
static PlatformContext* platform = NULL;
static PpuContext* ppu           = NULL;

// --- Timing, FPS, & Auto-Save Variables ---
static u64 lastFrameTime = 0;
static double timeAccumulator = 0.0;
static u32 framesThisSecond = 0;
static u64 lastFpsTime = 0;

static u32 autoSaveFrameCounter = 0;
static bool saveFileUpdated     = false; // Signalling flag for the JS database layer

// ============================================================================
// --- PURE STANDARD ANSI C FILE I/O SUBSYSTEM ---
// ============================================================================

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) {
    if (!RAM_DATA || RAM_SIZE == 0) return false;
    
    // Pure standard C file writing to the hardcoded virtual root path
    FILE* file = fopen("/game.sav", "wb");
    if (!file) return false;

    size_t written = fwrite(RAM_DATA, 1, RAM_SIZE, file);
    fclose(file);

    return (written == RAM_SIZE);
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) {
    if (!RAM_DATA || RAM_SIZE == 0) return false;

    // Pure standard C file reading from the hardcoded virtual root path
    FILE* file = fopen("/game.sav", "rb");
    if (!file) return false; // Returns false cleanly if no save file has been staged yet

    size_t readBytes = fread(RAM_DATA, 1, RAM_SIZE, file);
    fclose(file);

    return (readBytes == RAM_SIZE);
}

u32 fileGetExpectedSaveSize(void) {
    CartContext* ctx = cartridgeGetContext();
    if (!ctx || !ctx->initialized) return 0;
    
    // Account for 64-byte structural RTC offsets automatically if clock mappings exist
    return ctx->externalRamSize + (ctx->hasRTC ? 64 : 0);
}

// ============================================================================
// --- MAIN EXECUTION loop ---
// ============================================================================

void emscripten_main_loop(void) {
    u64 currentTime = SDL_GetTicks();
    if (lastFrameTime == 0) {
        lastFrameTime = currentTime;
        lastFpsTime = currentTime;
    }
    
    double deltaTime = (double)(currentTime - lastFrameTime);
    lastFrameTime = currentTime;

    if (deltaTime > 100.0) deltaTime = 100.0;
    timeAccumulator += deltaTime;

    double targetFrameTimeMs = 16.742;
    if (platform && platform->input.doubleSpeed) {
        targetFrameTimeMs /= 2.0; 
    }

    bool frameRendered = false;

    while (timeAccumulator >= targetFrameTimeMs) {
        if (platform && platform->input.poll) {
            platform->input.poll(&running);
        }

        if (!running) {
            emscripten_cancel_main_loop();
            return;
        }

        if (platform && !platform->input.paused) {
            int mCyclesThisFrame = 0;
            const int MAX_MCYCLES = 36000;

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

    if (frameRendered && platform) {
        if (platform->video.renderFrame) platform->video.renderFrame(&ppu->frameBuffer);
        if (platform->video.present) platform->video.present();
        
        framesThisSecond++;

        // --- BACKGROUND CORESYNC CONTROLLER ---
        // Every 5 seconds, check if the mappers marked the cartridge storage array as dirty
        autoSaveFrameCounter++;
        if (autoSaveFrameCounter >= 300) {
            autoSaveFrameCounter = 0;
            
            CartContext* ctx = cartridgeGetContext();
            if (ctx && ctx->initialized && ctx->ramDirty) {
                cartridgeFlushRAM();
                saveFileUpdated = true; // Signal flag raised for JavaScript poller
            }
        }
    }

    if (currentTime - lastFpsTime >= 1000) {
        framesThisSecond = 0;
        lastFpsTime = currentTime;
    }
}


// ============================================================================
// --- JAVASCRIPT EXPORTS (WASM API) ---
// ============================================================================

EMSCRIPTEN_KEEPALIVE
u8* webSaveState(u32* outSize) { return systemSaveStateToMemory(outSize); }

EMSCRIPTEN_KEEPALIVE
bool webLoadState(const u8* buffer, u32 size) { return systemLoadStateFromMemory(buffer, size); }

EMSCRIPTEN_KEEPALIVE
void webFreeStateBuffer(u8* buffer) { if (buffer) free(buffer); }

EMSCRIPTEN_KEEPALIVE
void webSetChannelVolumes(float ch1, float ch2, float ch3, float ch4) { apuSetChannelVolumes(ch1, ch2, ch3, ch4); }

EMSCRIPTEN_KEEPALIVE
void* webAllocate(u32 size) { return malloc(size); }

EMSCRIPTEN_KEEPALIVE
void webFree(void* ptr) { if (ptr) free(ptr); }

// Native Primitive Type Cast Pass-Throughs
EMSCRIPTEN_KEEPALIVE
void webSetPaused(i32 paused) { if (platform) platform->input.paused = (paused != 0); }

EMSCRIPTEN_KEEPALIVE
i32 webIsPaused(void) { return (platform && platform->input.paused) ? 1 : 0; }

// Core Save File Poller: Returns true if C has written changes to /game.sav
EMSCRIPTEN_KEEPALIVE
i32 webCheckAndClearSaveUpdated(void) {
    if (saveFileUpdated) {
        saveFileUpdated = false;
        return 1;
    }
    return 0;
}

static u32 current_c0 = 0xD1CB95FF;
static u32 current_c1 = 0x40985EFF;
static u32 current_c2 = 0x1A644EFF;
static u32 current_c3 = 0x04373BFF;

EMSCRIPTEN_KEEPALIVE
void webSetPalette(u32 c0, u32 c1, u32 c2, u32 c3) {
    current_c0 = c0; current_c1 = c1; current_c2 = c2; current_c3 = c3;
    if (platform && platform->video.setDmgPalette) {
        DmgPalette p = {c0, c1, c2, c3};
        platform->video.setDmgPalette(p);
    }
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
                    default: buffer[index] = 0xFFFF00FF; break;
                }
            } else {
                u8 r5 = (rawCoreData & 0x001F); 
                u8 g5 = (rawCoreData & 0x03E0) >> 5; 
                u8 b5 = (rawCoreData & 0x7C00) >> 10;
                
                u8 r8 = (r5 << 3) | (r5 >> 2); 
                u8 g8 = (g5 << 3) | (g5 >> 2); 
                u8 b8 = (b5 << 3) | (b5 >> 2);

                // FIXED: Swizzled channel output to guarantee perfect little-endian interpretation
                buffer[index] = (0xFF << 24) | (b8 << 16) | (g8 << 8) | r8;
            }
        }
    }
    return buffer;
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

    platform = platformGetContext();
    ppu = ppuGetContext();
    FORGE_LOG_INFO("%s", "--- POCKET PIXEL WEB STARTING ---");
    emscripten_set_main_loop(emscripten_main_loop, 0, 1);

    return 0;
}
