#include "cpu.h"
#include "cartridge.h"
#include "../../ForgeLib/include/asserts.h"
#include "../../ForgeLib/include/logger.h"

static bool         started = false;
static CPUContext   cpuCTX  = {0};
#define POSSIBLE_INSTRUCTION_COUNT 256


static Instruction instructions[POSSIBLE_INSTRUCTION_COUNT] =
    {
        // - - - Hex because easy to reference here: https://meganesulli.com/static/851d34afbc4673ee915a8233fda67922/78d47/opcode-tables-screenshot.png
        [0x00] = {INSTRUCTION_NOP,          ADDRESS_MODE_IMP},
        [0x05] = {INSTRUCTION_DECREMENT,    ADDRESS_MODE_R, REG_B},
        [0x0E] = {INSTRUCTION_LOAD,         ADDRESS_MODE_R_D8, REG_C},
        [0xAF] = {INSTRUCTION_XOR,          ADDRESS_MODE_R, REG_A},
        [0xC3] = {INSTRUCTION_JUMP,         ADDRESS_MODE_D16},
    };

FORGE_API void cpuInit()
{
    //FORGE_ASSERT_MESSAGE(!started, "Cannot start the emulator when it is already running");
    started = true;
    FORGE_LOG_INFO("Started the emulator");
}

FORGE_API void cpuTick()
{
    if (!cpuCTX.halted)
    {
        cpuCTX.currentOpcode    = cartridgeRead(cpuCTX.registerFile.programCounter++);
        cpuCTX.memDest          = 0;
        cpuCTX.destIsMemory     = false;

        switch (instructions[cpuCTX.currentOpcode].mode)
        {
            case ADDRESS_MODE_IMP   : return; // - - - Nothing needs to be read
//            default : TODO
        }
    }
}