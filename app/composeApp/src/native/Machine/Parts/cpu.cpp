#include "cpu.h"
#include "bus.h"
#include "cartridge.h"
#include "emu.h"
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
    [0xF3] = {INSTRUCTION_DI}
};

FORGE_API Instruction* InstructionByOpcode(u8 opCode)
{
    return &instructions[opCode];
}


// - - - CPU IMPLEMENTATIONS


// - - - CPU HELPER FUNCTIONS

// - - - MAIN CPU FUNCTIONS
FORGE_API void cpuInit()
{
    cpuCTX.registerFile.programCounter = 0x100;
    cpuCTX.registerFile.accumulator = 0x01;
    //FORGE_ASSERT_MESSAGE(!started, "Cannot start the emulator when it is already running");
    started = true;
    FORGE_LOG_INFO("Started the emulator");
}

FORGE_API bool cpuTick()
{
    if (!cpuCTX.halted)
    {
        cpuCTX.currentOpcode    = cartridgeRead(cpuCTX.registerFile.programCounter++);
        cpuCTX.memDest          = 0;
        cpuCTX.destIsMemory     = false;
        cpuCTX.currInstruction = InstructionByOpcode(cpuCTX.currentOpcode);

        switch (cpuCTX.currInstruction->mode) {
            case ADDRESS_MODE_IMP   :
                break; // - - - Nothing needs to be read
            case ADDRESS_MODE_R:
                // - - - manipulating current read data
                cpuCTX.readData = cpuReadRegister(
                        cpuCTX.currInstruction->reg1); //NOTE: IMPLEMENT CPU UTIL
            case ADDRESS_MODE_R_D8:
                cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
                emuCycles(1);
                cpuCTX.registerFile.programCounter++;
                break;
            case ADDRESS_MODE_D16:
            {
                u16 lo = busRead(cpuCTX.registerFile.programCounter);
                emuCycles(1);

                u16 hi = busRead(cpuCTX.registerFile.programCounter++);
                emuCycles(1);

                cpuCTX.readData = lo | (hi << 8);
                cpuCTX.registerFile.programCounter += 2;
                break;
            }
            default:
                FORGE_LOG_FATAL("HOW DID YOU EVEN END UP HERE DUMMY - Sak");
                exit(-7);
                break;
        }

    }
    if(cpuCTX.currInstruction == NULL)
    {
        FORGE_LOG_FATAL("UNKNOWN INSTRUCTION AT %02X\n", cpuCTX.currInstruction);
        exit(-7);
    }
    INSTRUCTION_PROCESS process = instructionGetProcessor(cpuCTX.currInstruction->type);
    if(!process)
    {
        FORGE_LOG_DEBUG("NO PROCESS GIVEN FROM CPU");
        exit(-7);
    }
    process(&cpuCTX);
    return true;
}