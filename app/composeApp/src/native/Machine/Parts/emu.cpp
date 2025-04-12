#include <cstdio>
#include "emu.h"
#include <thread>
#include "cartridge.h"
#include "cpu.h"
/*
  Emu components:

  |Cart|
  |CPU|
  |Address Bus|
  |PPU|
  |Timer|

*/

static emuContext ctx;

emuContext *emuGetContext() {
    return &ctx;
}

void delay(u32 ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
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
    // NOTE: NEED TO IMPLEMENT LOGIC FOR BUFFER IN KOTLIN
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
