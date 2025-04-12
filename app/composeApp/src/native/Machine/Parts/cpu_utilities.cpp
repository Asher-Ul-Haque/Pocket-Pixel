#include "cpu.h"
extern CPUContext cpuCTX;

u16 reverse(u16 n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 cpuReadRegister(RegisterType rt) {
    switch(rt) {
        case REG_A: return cpuCTX.registerFile.accumulator;
        case REG_F: return cpuCTX.registerFile.flags;
        case REG_B: return cpuCTX.registerFile.b;
        case REG_C: return cpuCTX.registerFile.c;
        case REG_D: return cpuCTX.registerFile.d;
        case REG_E: return cpuCTX.registerFile.e;
        case REG_H: return cpuCTX.registerFile.h;
        case REG_L: return cpuCTX.registerFile.l;

        case REG_AF: return reverse(*((u16 *)&cpuCTX.registerFile.accumulator));
        case REG_BC: return reverse(*((u16 *)&cpuCTX.registerFile.b));
        case REG_DE: return reverse(*((u16 *)&cpuCTX.registerFile.d));
        case REG_HL: return reverse(*((u16 *)&cpuCTX.registerFile.h));

        case REG_PC: return cpuCTX.registerFile.programCounter;
        case REG_SP: return cpuCTX.registerFile.stackPointer;
        default: return 0;
    }
}

void cpuSetRegister(RegisterType rt, u16 val) {
    switch(rt) {
        case REG_A: cpuCTX.registerFile.accumulator = val & 0xFF; break;
        case REG_F: cpuCTX.registerFile.flags = val & 0xFF; break;
        case REG_B: cpuCTX.registerFile.b = val & 0xFF; break;
        case REG_C: {
            cpuCTX.registerFile.c = val & 0xFF;
        } break;
        case REG_D: cpuCTX.registerFile.d = val & 0xFF; break;
        case REG_E: cpuCTX.registerFile.e = val & 0xFF; break;
        case REG_H: cpuCTX.registerFile.h = val & 0xFF; break;
        case REG_L: cpuCTX.registerFile.l = val & 0xFF; break;

        case REG_AF: *((u16 *)&cpuCTX.registerFile.accumulator) = reverse(val); break;
        case REG_BC: *((u16 *)&cpuCTX.registerFile.b) = reverse(val); break;
        case REG_DE: *((u16 *)&cpuCTX.registerFile.d) = reverse(val); break;
        case REG_HL: {
            *((u16 *)&cpuCTX.registerFile.h) = reverse(val);
            break;
        }

        case REG_PC: cpuCTX.registerFile.programCounter = val; break;
        case REG_SP: cpuCTX.registerFile.stackPointer = val; break;
        case REG_NONE: break;
    }
}

FORGE_API RegisterFile *cpuGetRegister()
{
    return &cpuCTX.registerFile;
}