#include "cpu.h"
#include "bus.h"
#include "cartridge.h"
#include "emu.h"
#include "stack.h"
#include "../../ForgeLib/include/asserts.h"
#include "../../ForgeLib/include/logger.h"

// - - - Process CPU instructions


// - - - Helper functions
static bool is16Bit(RegisterType rt)
{
    return rt>=REG_AF;
}

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

// - - - HELPER FUNCTION FOR JUMP INSTRUCTION
static void GOTO_ADDRESS(CPUContext* ctx, u16 addr, bool pushpc)
{
    if(checkCondition(ctx))
    {
        if(pushpc)
        {
            emuCycles(2); // - - - 16 BITS so 2 cycles
            stackPush16(ctx->registerFile.programCounter);
        }
    }
    ctx->registerFile.programCounter = addr;
    emuCycles(1);
}

// - - - CPU gives jump instruction
static void JUMP_PROCESS(CPUContext* ctx)
{
    GOTO_ADDRESS(ctx, ctx->readData, false);
}

// - - - CPU gives JUMP with PUSH (CALL FUNCTION)
static void CALL_PROCESS(CPUContext* ctx)
{
    GOTO_ADDRESS(ctx, ctx->readData, true);
}

// - - - CPU gives JUMP_R
static void JR_PROCESS(CPUContext* ctx)
{
    char rel = (char)(ctx->readData & 0xFF);
    u16 addr = ctx->registerFile.programCounter + rel;
    GOTO_ADDRESS(ctx, addr, false);
}

static void RST_PROCESS(CPUContext* ctx)
{
    GOTO_ADDRESS(ctx, ctx->currInstruction->param, true);
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


// - - -  CPU gives STATIC LOAD
static void LDH_PROCESS(CPUContext* ctx)
{
    if(ctx->currInstruction->reg1 == REG_A)
    {
        cpuSetRegister(ctx->currInstruction->reg1, busRead(0xFF00 | ctx->readData));
    }
    else
    {
        busWrite(0xFF00|ctx->readData, ctx->registerFile.accumulator);
    }
    emuCycles(1);
}


static void PUSH_PROCESS(CPUContext* ctx)
{
    u16 hi = (cpuReadRegister(ctx->currInstruction->reg1) >> 8) & 0xFF;
    emuCycles(1);
    stackPush(hi);

    u16 lo = cpuReadRegister(ctx->currInstruction->reg1) & 0xFF;
    emuCycles(1);
    stackPush(lo);

    emuCycles(1);
}



static void POP_PROCESS(CPUContext* ctx)
{
    u16 lo = stackPop();
    emuCycles(1);
    u16 hi = stackPop();
    emuCycles(1);

    u16 n = (hi << 8) | lo;
    cpuSetRegister(ctx->currInstruction->reg1, n);

    if(ctx->currInstruction->reg1 == REG_AF)
    {
        cpuSetRegister(ctx->currInstruction->reg1, n & 0xFFF0);
    }
}

// - - - When the CPU returns to a register
static void RET_PROCESS(CPUContext* ctx)
{
    if(ctx->currInstruction->cond != CONDITIONS_NONE)
    {
        emuCycles(1);
    }
    if(checkCondition(ctx))
    {
        u16 lo = stackPop();
        emuCycles(1);
        u16 hi = stackPop();
        emuCycles(2);

        u16 n = (hi << 8) | lo;
        ctx->registerFile.programCounter = n;

        emuCycles(1);
    }
}

// - - - Renable the master Reg
static void RETI_PROCESS(CPUContext* ctx)
{
    ctx->instructionMasterEnabled = true;
    RET_PROCESS(ctx);
}


// - - - ARITHEMATIC INSTRUCTIONS

static void SUB_PROCESS(CPUContext* ctx)
{
    u16 val = cpuReadRegister(ctx->currInstruction->reg1) - ctx->readData;

    int z = val == 0;
    int h = ((int)cpuReadRegister(ctx->currInstruction->reg1) & 0xF) - ((int)ctx->readData & 0xF) < 0;
    int c = ((int)cpuReadRegister(ctx->currInstruction->reg1)) - ((int)ctx->readData) < 0;

    cpuSetRegister(ctx->currInstruction->reg1, val);
    cpuSetFlag(ctx, z, 1, h, c);
}

static void SBC_PROCESS(CPUContext* ctx)
{
    u8 val = ctx->readData + CPU_FLAG_C;

    int z = cpuReadRegister(ctx->currInstruction->reg1) - val == 0;
    int h = ((int)cpuReadRegister(ctx->currInstruction->reg1) & 0xF) - ((int)ctx->readData & 0xF) - ((int)CPU_FLAG_C) < 0;
    int c = ((int)cpuReadRegister(ctx->currInstruction->reg1)) - ((int)ctx->readData ) - ((int)CPU_FLAG_C) < 0;
    cpuSetRegister(ctx->currInstruction->reg1, cpuReadRegister(ctx->currInstruction->reg1) - val);
    cpuSetFlag(ctx, z, 1, h, c);
}

static void ADC_PROCESS(CPUContext* ctx)
{
    u16 u = ctx->readData;
    u16 a = ctx->registerFile.accumulator;
    u16 c = CPU_FLAG_C;

    ctx->registerFile.accumulator = (a+u+c) && 0xFF;

    cpuSetFlag(ctx, ctx->registerFile.accumulator == 0, 0, (a&0xF) + (u&0xF) + c > 0xF, a+ u + c > 0xFF);
}

static void ADD_PROCESS(CPUContext* ctx)
{
    u32 val = cpuReadRegister(ctx->currInstruction->reg1) + ctx->readData;
    bool is16 = is16Bit(ctx->currInstruction->reg1);
    if(is16){ emuCycles(1);}
    if(ctx->currInstruction->reg1 == REG_SP)
    {
        val = cpuReadRegister(ctx->currInstruction->reg1) + (char)ctx->readData;
    }

    int z = (val&0xFF) == 0;
    int h = (cpuReadRegister(ctx->currInstruction->reg1) & 0xF) + (ctx->readData & 0xF) >= 0x10;
    int c = (int)(cpuReadRegister(ctx->currInstruction->reg1) & 0xFF) + (int)(ctx->readData & 0xFF) >= 0x100;

    if(is16)
    {
        z=-1;
        h = (cpuReadRegister(ctx->currInstruction->reg1)&0xFFF) + (ctx->readData & 0xFFF)>= 0x1000;
        u32 n = ((u32) cpuReadRegister(ctx->currInstruction->reg1)) + ((u32)ctx->readData);
        c = n>=0x10000;
    }

    if(ctx->currInstruction->reg1 == REG_SP)
    {
        z = 0;
        h = (cpuReadRegister(ctx->currInstruction->reg1) & 0xF) + (ctx->readData & 0xF) >= 0x10;
        c = (int)(cpuReadRegister(ctx->currInstruction->reg1) & 0xFF) + (int)(ctx->readData & 0xFF) >= 0x100;
    }

    cpuSetRegister(ctx->currInstruction->reg1, val & 0xFFFF);
    cpuSetFlag(ctx,z,0,h,c);
}

static void INCREMENT_PROCESS(CPUContext* ctx)
{
    u16 val = cpuReadRegister(ctx->currInstruction->reg1) + 1;
    if(is16Bit(ctx->currInstruction->reg1))
    {
        emuCycles(1);
    }
    if(ctx->currInstruction->reg1 == REG_HL && ctx->currInstruction->mode == ADDRESS_MODE_MR)
    {
        val = busRead(cpuReadRegister(REG_HL))+1;
        val&= 0xFF;
        busWrite(cpuReadRegister(REG_HL), val);
    }
    else
    {
        cpuSetRegister(ctx->currInstruction->reg1, val);
        val = cpuReadRegister(ctx->currInstruction->reg1);
    }

    if((ctx->currentOpcode & 0x03) == 0x03) {return;}
    cpuSetFlag(ctx, val == 0, 0, (val& 0x0F), -1);
}

static void DECREMENT_PROCESS(CPUContext* ctx)
{
    u16 val = cpuReadRegister(ctx->currInstruction->reg1) - 1;
    if(is16Bit(ctx->currInstruction->reg1))
    {
        emuCycles(1);
    }
    if(ctx->currInstruction->reg1 == REG_HL && ctx->currInstruction->mode == ADDRESS_MODE_MR)
    {
        val = busRead(cpuReadRegister(REG_HL))-1;
        busWrite(cpuReadRegister(REG_HL), val);
    }
    else
    {
        cpuSetRegister(ctx->currInstruction->reg1, val);
        val = cpuReadRegister(ctx->currInstruction->reg1);
    }

    if((ctx->currentOpcode & 0x0B) == 0x0B) {return;}
    cpuSetFlag(ctx, val == 0, 1, (val& 0x0F) == 0x0F, -1);
}

// - - - fix bad identation later thanks to Andriod Development studio
static INSTRUCTION_PROCESS process[] =
        {
               [INSTRUCTION_NONE]  = NONE_PROCESS,
               [INSTRUCTION_NOP]   = NOP_PROCESS,
               [INSTRUCTION_LOAD]  = LOAD_PROCESS,
               [INSTRUCTION_LDH]   = LDH_PROCESS,
               [INSTRUCTION_JUMP]  = JUMP_PROCESS,
               [INSTRUCTION_DI]    = DI_PROCESS,
               [INSTRUCTION_POP]   = POP_PROCESS,
               [INSTRUCTION_PUSH]  = PUSH_PROCESS,
               [INSTRUCTION_JR]    = JR_PROCESS,
               [INSTRUCTION_CALL]  = CALL_PROCESS,
               [INSTRUCTION_RET]   = RET_PROCESS,
               [INSTRUCTION_RST]   = RST_PROCESS,
               [INSTRUCTION_DECREMENT] = DECREMENT_PROCESS,
               [INSTRUCTION_INCREMENT] = INCREMENT_PROCESS,
               [INSTRUCTION_ADD]   = ADD_PROCESS,
               [INSTRUCTION_ADC]   = ADC_PROCESS,
               [INSTRUCTION_SUB]   = SUB_PROCESS,
               [INSTRUCTION_SBC]   = SBC_PROCESS,
               [INSTRUCTION_RETI]  = RETI_PROCESS,
               [INSTRUCTION_XOR]   = XOR_PROCESS,

        };

INSTRUCTION_PROCESS instructionGetProcessor(InstructionType type)
{
    return process[type];
}