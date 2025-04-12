#include <cstdio>
#include "emu.h"
#include "cartridge.h"
#include "cpu.h"
#include "SDL2/SDL.h"
#include "SDL2/"
/*
  Emu components:

  |Cart|
  |CPU|
  |Address Bus|
  |PPU|
  |Timer|

*/

static emuContext ctx;

emuContext *emu_get_context() {
    return &ctx;
}

void delay(u32 ms) {
    SDL_Delay(ms);
}

int emuRunning(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: emu <rom_file>, <rom_size>\n");
        return -1;
    }

    if (!cartridgeLoad(argv[1], argv[2])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }

    printf("Cart loaded..\n");

    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    TTF_Init();
    printf("TTF INIT\n");

    cpuInit();

    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    while(ctx.running) {
        if (ctx.paused) {
            delay(10);
            continue;
        }

        if (!cpuTick()) {
            printf("CPU Stopped\n");
            return -3;
        }

        ctx.ticks++;
    }

    return 0;
}

void emuCycles(int cpu_cycles) {
    //TODO...
}
