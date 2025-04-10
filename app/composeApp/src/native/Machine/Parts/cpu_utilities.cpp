#include "cpu.h"
extern CPUContext ctx;

u16 reverse(u16 n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 cpuReadRegister(RegisterType rt) {
    switch(rt) {
        case REG_A: return ctx.registerFile.accumulator;
        case REG_F: return ctx.registerFile.flags;
        case REG_B: return ctx.registerFile.b;
        case REG_C: return ctx.registerFile.c;
        case REG_D: return ctx.registerFile.d;
        case REG_E: return ctx.registerFile.e;
        case REG_H: return ctx.registerFile.h;
        case REG_L: return ctx.registerFile.l;

        case REG_AF: return reverse(*((u16 *)&ctx.registerFile.accumulator));
        case REG_BC: return reverse(*((u16 *)&ctx.registerFile.b));
        case REG_DE: return reverse(*((u16 *)&ctx.registerFile.d));
        case REG_HL: return reverse(*((u16 *)&ctx.registerFile.h));

        case REG_PC: return ctx.registerFile.programCounter;
        case REG_SP: return ctx.registerFile.stackPointer;
        default: return 0;
    }
}

void cpuSetRegister(RegisterType rt, u16 val) {
    switch(rt) {
        case REG_A: ctx.registerFile.accumulator = val & 0xFF; break;
        case REG_F: ctx.registerFile.flags = val & 0xFF; break;
        case REG_B: ctx.registerFile.b = val & 0xFF; break;
        case REG_C: {
            ctx.registerFile.c = val & 0xFF;
        } break;
        case REG_D: ctx.registerFile.d = val & 0xFF; break;
        case REG_E: ctx.registerFile.e = val & 0xFF; break;
        case REG_H: ctx.registerFile.h = val & 0xFF; break;
        case REG_L: ctx.registerFile.l = val & 0xFF; break;

        case REG_AF: *((u16 *)&ctx.registerFile.accumulator) = reverse(val); break;
        case REG_BC: *((u16 *)&ctx.registerFile.b) = reverse(val); break;
        case REG_DE: *((u16 *)&ctx.registerFile.d) = reverse(val); break;
        case REG_HL: {
            *((u16 *)&ctx.registerFile.h) = reverse(val);
            break;
        }

        case REG_PC: ctx.registerFile.programCounter = val; break;
        case REG_SP: ctx.registerFile.stackPointer = val; break;
        case REG_NONE: break;
    }
}