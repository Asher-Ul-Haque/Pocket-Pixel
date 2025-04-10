#include "cpu.h"
#include "bus.h"
#include "cartridge.h"
#include "emu.h"
#include "../../ForgeLib/include/asserts.h"
#include "../../ForgeLib/include/logger.h"

// - - - Process CPU instructions


// - - - Helper functions

static bool checkCondition(CPUContext* ctx)
{
    bool z = CPU_FLAG_Z;
    bool c = CPU_FLAG_C;
    switch(ctx->currInstruction->cond)
    {
        case CONDITIONS_NONE: return true;
        case CONDITIONS_C: return c;
        case CONDITIONS_NC: return !c;
        case CONDITIONS_NZ: return !z;
        case CONDITIONS_Z: return z;
    }
    FORGE_LOG_TRACE("NONE CONDITION FOR C, Z RECIEVED. OUTSIDE SWITCH");
    return false;
}


static void cpuSetFlag(CPUContext *ctx, char z, char n, char h, char c)
{
    if(z!= -1)
    {
        SET_BIT(ctx->registerFile.flags, 7, z);
    }

    if(n!= -1)
    {
        SET_BIT(ctx->registerFile.flags, 6, n);
    }

    if(h!= 1)
    {
        SET_BIT(ctx->registerFile.flags, 5, h);
    }

    if(c!=-1)
    {
        SET_BIT(ctx->registerFile.flags, 4, c);
    }

}
// - - - CPU gives a null process
static void NONE_PROCESS(CPUContext* ctx)
{
    FORGE_LOG_INFO("NOT A VALID INSTRUCTION");
    exit(-7);
}

// - - - CPU gives load instruction
static void LOAD_PROCESS(CPUContext* ctx) {
    if (ctx->destIsMemory) {
        if (ctx->currInstruction->reg2 >= REG_AF) {
            emuCycles(1);
            busWrite16(ctx->memDest, ctx->readData);
        } else {
            busWrite(ctx->memDest, ctx->readData);
        }
        return;
    }
    if (ctx->currInstruction->mode == ADDRESS_MODE_HL_SPR)
    {
        u8 hflag = (cpuReadRegister(ctx->currInstruction->reg2) & 0xF)+(ctx->readData&0xF) >= 0x10;
        u8 cflag = (cpuReadRegister(ctx->currInstruction->reg2) & 0xFF)+(ctx->readData&0xF) >= 0x100;
        cpuSetFlag(ctx, 0,0,hflag,cflag);
        cpuSetRegister(ctx->currInstruction->reg1, cpuReadRegister(ctx->currInstruction->reg2) + (char)ctx->readData);
        return;
    }
    cpuSetRegister(ctx->currInstruction->reg1, ctx->readData);
}

// - - - CPU gives jump instruction
static void JUMP_PROCESS(CPUContext* ctx)
{
    if(checkCondition(ctx))
    {
        ctx->registerFile.programCounter = ctx->readData;
        emuCycles(1);

    }
}

// - - - CPU gives NOP instruction
static void NOP_PROCESS(CPUContext* ctx)
{

}

// - - - CPU gives DI instruction
static void DI_PROCESS(CPUContext* ctx)
{
    ctx->instructionMasterEnabled = false;
}

// - - - CPU gives XOR instruction
static void XOR_PROCESS(CPUContext* ctx)
{
    ctx->registerFile.accumulator ^= ctx->readData & 0xFF;
    cpuSetFlag(ctx, ctx->registerFile.accumulator == 0,0,0,0);
}

// - - - fix bad identation later thanks to Andriod Development studio
static INSTRUCTION_PROCESS process[] =
        {
               [INSTRUCTION_NONE] =  NONE_PROCESS,
               [INSTRUCTION_NOP] = NOP_PROCESS,
               [INSTRUCTION_DI] = DI_PROCESS,
               [INSTRUCTION_XOR] = XOR_PROCESS,
               [INSTRUCTION_LOAD] = LOAD_PROCESS,
               [INSTRUCTION_JUMP] = JUMP_PROCESS,

        };

INSTRUCTION_PROCESS instructionGetProcess(InstructionType type)
{
    return process[type];
}