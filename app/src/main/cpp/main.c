#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <common.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <ppu/dma.h>
#include <cartridge/cartridge.h>
#include <utils/logger.h>
#include <SDL3/SDL.h>

// --- Static Platform Helpers ---

static char g_savePath[1024];

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) {
    if (g_savePath[0] == '\0') return false;
    FILE* f = fopen(g_savePath, "wb");
    if (!f) return false;
    size_t written = fwrite(RAM_DATA, 1, RAM_SIZE, f);
    fclose(f);
    return written == RAM_SIZE;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) {
    if (g_savePath[0] == '\0') return false;
    FILE* f = fopen(g_savePath, "rb");
    if (!f) return false;
    size_t read = fread(RAM_DATA, 1, RAM_SIZE, f);
    fclose(f);
    return read == RAM_SIZE;
}

u32 fileGetExpectedSaveSize(void) {
    return 0; 
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        FORGE_LOG_FATAL("Usage: %s <rom_file>", argv[0]);
        return 1;
    }

    const char* romPath = argv[1];

    // 1. Read ROM file into memory
    FILE* f = fopen(romPath, "rb");
    if (!f) {
        FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long romSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8* romData = (u8*)malloc(romSize);
    if (!romData) {
        FORGE_LOG_FATAL("%s", "Memory allocation failed for ROM data");
        fclose(f);
        return 1;
    }

    if (fread(romData, 1, romSize, f) != (size_t)romSize) {
        FORGE_LOG_ERROR("%s", "Failed to read ROM data");
        free(romData);
        fclose(f);
        return 1;
    }
    fclose(f);

    snprintf(g_savePath, sizeof(g_savePath), "%s.sav", romPath);

    CartridgeFileIO fileIO = {
        .saveRamToFile = fileSaveRam,
        .loadRamFromFile = fileLoadRam,
        .getExpectedSaveSize = fileGetExpectedSaveSize
    };

    // 2. Initialize Core Components
    if (!cartridgeInit(&fileIO, romData, (u32)romSize)) {
        FORGE_LOG_FATAL("%s", "Failed to initialize cartridge");
        free(romData);
        return 1;
    }

    free(romData); 

    platformInit();
    PlatformContext* platform = platformGetContext();

    ppuInit();
    dmaInit();
    cpuInit();

    // Skip Boot ROM and jump to entry point
    CpuContext* cpu = cpuGetContext();
    cpu->registers.programCounter = 0x0100;

    FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");

    bool running = true;
    SDL_Event event;

    while (running) {
        // A. Poll Events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // B. Core Tick Logic
        // Only log when a new instruction starts to avoid <no-instr> spam
        if (cpu->state == CPU_STATE_FETCH && cpu->mCycleInInstr == 0) {
            char debugInfo[256];
            cpuTraceLineToString(cpu->registers.programCounter, debugInfo, sizeof(debugInfo));
            FORGE_LOG_DEBUG("%s", debugInfo);
        }

        cpuTick();  // Execute 1 M-Cycle
        dmaTick();  // Handle OAM DMA if active

        // PPU ticks at T-cycle resolution (4 per M-cycle)
        for (int i = 0; i < 4; i++) {
            ppuMTick();
        }

        // C. Rendering Sync
        PpuContext* ppu = ppuGetContext();
        
        // Render when PPU finishes a frame (entering V-Blank)
        // Transition from LY 143 to 144 is the standard trigger
        static u8 last_ly = 0;
        if (ppu->ly == 144 && last_ly == 143) {
            platform->rendering.renderFrame(ppu->frameBuffer, 160, 144);
        }
        last_ly = ppu->ly;
    }

    platform->rendering.cleanup();
    return 0;
}
