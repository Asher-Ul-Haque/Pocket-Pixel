#include "cpu.h"
#include "bus.h"
#include "emu.h"

extern CPUContext cpuCTX;

void readData() {
    cpuCTX.memDest = 0;
    cpuCTX.destIsMemory = false;

    if (cpuCTX.currInstruction == NULL) {
        return;
    }

    switch(cpuCTX.currInstruction->mode) {
        case ADDRESS_MODE_IMP:
            return;

        case ADDRESS_MODE_R:
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg1);
            return;

        case ADDRESS_MODE_R_R:
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg2);
            return;

        case ADDRESS_MODE_R_D8:
            cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_R_D16:
        case ADDRESS_MODE_D16: {
            u16 lo = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
            emuCycles(1);

            cpuCTX.readData = lo | (hi << 8);
            cpuCTX.registerFile.programCounter += 2;
            return;
        }

        case ADDRESS_MODE_MR_R:
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg2);
            cpuCTX.memDest = cpuReadRegister(cpuCTX.currInstruction->reg1);
            cpuCTX.destIsMemory = true;

            if (cpuCTX.currInstruction->reg1 == REG_C) {
                cpuCTX.memDest |= 0xFF00;
            }
            return;

        case ADDRESS_MODE_R_MR: {
            u16 addr = cpuReadRegister(cpuCTX.currInstruction->reg2);

            if (cpuCTX.currInstruction->reg2 == REG_C) {
                addr |= 0xFF00;
            }

            cpuCTX.readData = busRead(addr);
            emuCycles(1);
            return;
        }

        case ADDRESS_MODE_R_HLI:
            cpuCTX.readData = busRead(cpuReadRegister(cpuCTX.currInstruction->reg2));
            emuCycles(1);
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
            return;

        case ADDRESS_MODE_R_HLD:
            cpuCTX.readData = busRead(cpuReadRegister(cpuCTX.currInstruction->reg2));
            emuCycles(1);
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
            return;

        case ADDRESS_MODE_HLI_R:
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg2);
            cpuCTX.memDest = cpuReadRegister(cpuCTX.currInstruction->reg1);
            cpuCTX.destIsMemory = true;
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
            return;

        case ADDRESS_MODE_HLD_R:
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg2);
            cpuCTX.memDest = cpuReadRegister(cpuCTX.currInstruction->reg1);
            cpuCTX.destIsMemory = true;
            cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
            return;

        case ADDRESS_MODE_R_A8:
            cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_A8_R:
            cpuCTX.memDest = busRead(cpuCTX.registerFile.programCounter) | 0xFF00;
            cpuCTX.destIsMemory = true;
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_HL_SPR:
            cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_D8:
            cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            return;

        case ADDRESS_MODE_A16_R:
        case ADDRESS_MODE_D16_R: {
            u16 lo = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
            emuCycles(1);

            cpuCTX.memDest = lo | (hi << 8);
            cpuCTX.destIsMemory = true;

            cpuCTX.registerFile.programCounter += 2;
            cpuCTX.readData = cpuReadRegister(cpuCTX.currInstruction->reg2);
            return;
        }

        case ADDRESS_MODE_MR_D8:
            cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);
            cpuCTX.registerFile.programCounter++;
            cpuCTX.memDest = cpuReadRegister(cpuCTX.currInstruction->reg1);
            cpuCTX.destIsMemory = true;
            return;

        case ADDRESS_MODE_MR:
            cpuCTX.memDest = cpuReadRegister(cpuCTX.currInstruction->reg1);
            cpuCTX.destIsMemory = true;
            cpuCTX.readData = busRead(cpuReadRegister(cpuCTX.currInstruction->reg1));
            emuCycles(1);
            return;

        case ADDRESS_MODE_R_A16: {
            u16 lo = busRead(cpuCTX.registerFile.programCounter);
            emuCycles(1);

            u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
            emuCycles(1);

            u16 addr = lo | (hi << 8);

            cpuCTX.registerFile.programCounter += 2;
            cpuCTX.readData = busRead(addr);
            emuCycles(1);
            return;
        }

        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", cpuCTX.currInstruction->mode, cpuCTX.currentOpcode);
            exit(-7);
            return;
    }
}
