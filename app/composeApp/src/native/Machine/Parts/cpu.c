#include "cpu.h"
#include "../../ForgeLib/include/asserts.h"
#include "../../ForgeLib/include/logger.h"

static bool started = false;

FORGE_API void cpuInit()
{
    FORGE_ASSERT_MESSAGE(!started, "Cannot start the emulator when it is already running");
    started = true;
    FORGE_LOG_INFO("Started the emulator");
}

FORGE_API void cpuTick()
{
//    TODO
}