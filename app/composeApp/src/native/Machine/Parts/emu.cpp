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

int emuRunning(u8* rom, u64 romSize) {
    if (!cartridgeLoad(rom, romSize)) {
        printf("Failed to load ROM file at EMU RUNNING");
        return -2;
    }

    FORGE_LOG_DEBUG("Cart loaded...");
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
