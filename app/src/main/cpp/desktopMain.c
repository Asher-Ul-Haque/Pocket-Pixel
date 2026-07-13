// desktopMain.c
#include "apu/apu.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <joypad.h>

#define TARGET_FRAME_MS_60FPS  16
#define TARGET_FRAME_MS_120FPS 8

static char savePath[1024];

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) {
    if (savePath[0] == '\0') return false;
    FILE* file = fopen(savePath, "wb");
    if (!file) return false;
    u64 written = fwrite(RAM_DATA, 1, RAM_SIZE, file);
    fclose(file);
    return written == RAM_SIZE;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) {
    if (savePath[0] == '\0') return false;
    FILE* file = fopen(savePath, "rb");
    if (!file) return false;
    u64 read = fread(RAM_DATA, 1, RAM_SIZE, file);
    fclose(file);
    return read == RAM_SIZE;
}

u32 fileGetExpectedSaveSize(void) { return 0; }

i32 main(int ARGUMENT_COUNT, char* ARGUMENT_VECTOR[]) {
    if (ARGUMENT_COUNT < 2) {
        FORGE_LOG_FATAL("Usage: %s <rom_file>", ARGUMENT_VECTOR[0]);
        return 1;
    }

    const char* romPath = ARGUMENT_VECTOR[1];
    FILE* file = fopen(romPath, "rb");
    if (!file) return 1;

    fseek(file, 0, SEEK_END);
    u64 romSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (romSize == 0 || romSize > 64 * 1024 * 1024) {
        fclose(file);
        return 1;
    }

    u8* romData = (u8*)malloc(romSize);
    if (!romData) {
        fclose(file);
        return 1;
    }

    if (fread(romData, 1, (size_t)romSize, file) != romSize) {
        free(romData);
        fclose(file);
        return 1;
    }
    fclose(file);
    snprintf(savePath, sizeof(savePath), "%s.sav", romPath);

    CartridgeFileIO fileIO = {
        .saveRamToFile       = fileSaveRam,
        .loadRamFromFile     = fileLoadRam,
        .getExpectedSaveSize = fileGetExpectedSaveSize
    };

    if (!cartridgeInit(&fileIO, romData, (u32)romSize)) {
        FORGE_LOG_FATAL("%s", "Failed to initialize cartridge");
        free(romData);
        return 1;
    }

    platformInit();
    joypadInit();
    cpuInit();
    ppuInit();
    apuInit();
    timerInit();

    PlatformContext* platform = platformGetContext();

    FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");

    bool running      = true;
    bool paused       = false;
    bool doubleSpeed  = false;
    bool showVram     = false;
    bool pKeyHeld     = false;
    bool fKeyHeld     = false;
    bool tKeyHeld     = false;

    PpuContext* ppu = ppuGetContext();

    while (running) {
        u64 frameStartMs = SDL_GetTicks();

        if (platform && platform->input.poll) {
            platform->input.poll(&running);
        }

        if (!running) break;

        const bool* keyboard = SDL_GetKeyboardState(NULL);

        bool pPressed = keyboard[SDL_SCANCODE_P];
        if (pPressed && !pKeyHeld) {
            paused = !paused;
            FORGE_LOG_INFO("Emulation %s", paused ? "PAUSED" : "RESUMED");
        }
        pKeyHeld = pPressed;

        bool fPressed = keyboard[SDL_SCANCODE_F];
        if (fPressed && !fKeyHeld) {
            doubleSpeed = !doubleSpeed;
            if (doubleSpeed) apuSetSpeed(2.0f);
            else apuSetSpeed(1.0f);
        }
        fKeyHeld = fPressed;

        bool tPressed = keyboard[SDL_SCANCODE_T];
        if (tPressed && !tKeyHeld) {
            showVram = !showVram;
            extern bool gDebugWindowOpen;
            gDebugWindowOpen = showVram;
        }
        tKeyHeld = tPressed;

        if (!paused) {
            while (running && !ppu->frameReady) {
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
            }
            ppu->frameReady = false;
        }

        if (platform) {
            if (platform->video.renderFrame) platform->video.renderFrame(&ppu->frameBuffer);
            if (showVram && platform->video.drawTileView) platform->video.drawTileView(ppu->vram[0], ppu->vram[1]);
            if (platform->video.present) platform->video.present();
        }

        const u32 targetFrameMs = doubleSpeed ? TARGET_FRAME_MS_120FPS : TARGET_FRAME_MS_60FPS;
        u64 frameEndMs = SDL_GetTicks();
        u64 frameDurationMs = frameEndMs - frameStartMs;

        if (frameDurationMs < targetFrameMs) {
            SDL_Delay((u32)(targetFrameMs - frameDurationMs));
        }
    }

    if (platform && platform->video.cleanup) platform->video.cleanup();
    
    cartridgeUnload();
    free(romData);
    return 0;
}
