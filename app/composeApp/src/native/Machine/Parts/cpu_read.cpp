#include "cpu.h"
#include "bus.h"
#include "emu.h"

extern CPUContext ctx;

void readData() {
    ctx.memDest = 0;
    ctx.destIsMemory = false;

    if (ctx.currInstruction == NULL) {
        return;
    }

    switch(ctx.currInstruction->mode) {
        case ADDRESS_MODE_IMP:
            return;

        case ADDRESS_MODE_R:
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg1);
            return;

        case ADDRESS_MODE_R_R:
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg2);
            return;

        case ADDRESS_MODE_R_D8:
            ctx.readData = busRead(ctx.registerFile.programCounter);
            emuCycles(1);
            ctx.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_R_D16:
        case ADDRESS_MODE_D16: {
            u16 lo = busRead(ctx.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(ctx.registerFile.programCounter + 1);
            emuCycles(1);

            ctx.readData = lo | (hi << 8);
            ctx.registerFile.programCounter += 2;
            return;
        }

        case ADDRESS_MODE_MR_R:
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg2);
            ctx.memDest = cpuReadRegister(ctx.currInstruction->reg1);
            ctx.destIsMemory = true;

            if (ctx.currInstruction->reg1 == REG_C) {
                ctx.memDest |= 0xFF00;
            }
            return;

        case ADDRESS_MODE_R_MR: {
            u16 addr = cpuReadRegister(ctx.currInstruction->reg2);

            if (ctx.currInstruction->reg2 == REG_C) {
                addr |= 0xFF00;
            }

            ctx.readData = busRead(addr);
            emuCycles(1);
            return;
        }

        case ADDRESS_MODE_R_HLI:
            ctx.readData = busRead(cpuReadRegister(ctx.currInstruction->reg2));
            emuCycles(1);
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
            return;

        case ADDRESS_MODE_R_HLD:
            ctx.readData = busRead(cpuReadRegister(ctx.currInstruction->reg2));
            emuCycles(1);
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
            return;

        case ADDRESS_MODE_HLI_R:
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg2);
            ctx.memDest = cpuReadRegister(ctx.currInstruction->reg1);
            ctx.destIsMemory = true;
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
            return;

        case ADDRESS_MODE_HLD_R:
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg2);
            ctx.memDest = cpuReadRegister(ctx.currInstruction->reg1);
            ctx.destIsMemory = true;
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
            return;

        case ADDRESS_MODE_R_A8:
            ctx.readData = busRead(ctx.registerFile.programCounter);
            emuCycles(1);
            ctx.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_A8_R:
            ctx.memDest = busRead(ctx.registerFile.programCounter) | 0xFF00;
            ctx.destIsMemory = true;
            emuCycles(1);
            ctx.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_HL_SPR:
            ctx.readData = busRead(ctx.registerFile.programCounter);
            emuCycles(1);
            ctx.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_D8:
            ctx.readData = busRead(ctx.registerFile.programCounter);
            emuCycles(1);
            ctx.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_A16_R:
        case ADDRESS_MODE_D16_R: {
            u16 lo = busRead(ctx.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(ctx.registerFile.programCounter + 1);
            emuCycles(1);

            ctx.memDest = lo | (hi << 8);
            ctx.destIsMemory = true;

            ctx.registerFile.programCounter += 2;
            ctx.readData = cpuReadRegister(ctx.currInstruction->reg2);
            return;
        }

        case ADDRESS_MODE_MR_D8:
            ctx.readData = busRead(ctx.registerFile.programCounter);
            emuCycles(1);
            ctx.registerFile.programCounter++;
            ctx.memDest = cpuReadRegister(ctx.currInstruction->reg1);
            ctx.destIsMemory = true;
            return;

        case ADDRESS_MODE_MR:
            ctx.memDest = cpuReadRegister(ctx.currInstruction->reg1);
            ctx.destIsMemory = true;
            ctx.readData = busRead(cpuReadRegister(ctx.currInstruction->reg1));
            emuCycles(1);
            return;

        case ADDRESS_MODE_R_A16: {
            u16 lo = busRead(ctx.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(ctx.registerFile.programCounter + 1);
            emuCycles(1);

            u16 addr = lo | (hi << 8);

            ctx.registerFile.programCounter += 2;
            ctx.readData = busRead(addr);
            emuCycles(1);
            return;
        }

        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", ctx.currInstruction->mode, ctx.currentOpcode);
            exit(-7);
            return;
    }
}
