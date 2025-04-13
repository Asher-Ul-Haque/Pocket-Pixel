#include <cstdio>
#include "emu.h"
#include <thread>
#include "cartridge.h"
#include "ui.h"
#include <pthread.h>
#include <unistd.h>
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

// - - - Main Thread
void* cpuRun(void *p)
{
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
            return 0;
        }
        ctx.ticks++;
    }
    return 0;
}

int emuRunning(u8* rom, u64 romSize) {
    if (!cartridgeLoad(rom, romSize)) {
        printf("Failed to load ROM file at EMU RUNNING");
        return -2;
    }

    FORGE_LOG_DEBUG("Cart loaded...");
    // NOTE: NEED TO IMPLEMENT LOGIC FOR BUFFER IN KOTLIN
    ui_init();
    pthread_t t1;
    if(pthread_create(&t1, NULL, cpuRun, NULL))
    {
        FORGE_LOG_ERROR("THREAD CREATION FAILED FOR CPU");
        exit(-1);
    }

    while(!ctx.die)
    {
        usleep(1000);
        uiHandleEvents();
    }

    return 0;
}

void emuCycles(int cpu_cycles) {
    //TODO...
}
