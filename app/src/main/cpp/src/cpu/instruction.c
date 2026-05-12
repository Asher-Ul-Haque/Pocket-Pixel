#include <cpu/cpu.h>
#include <cpu/ops.h>
#include <cpu/instruction.h>

static Instruction opTable[INSTRUCTION_COUNT] = 
  {
    // - - - 8 bit loads reg to reg 
    [OP_LOAD_B_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_B, .reg1 = RT_B, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_C, .reg1 = RT_B, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_D, .reg1 = RT_B, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_E, .reg1 = RT_B, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_H, .reg1 = RT_B, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_L, .reg1 = RT_B, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_B_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_B_A, .reg1 = RT_B, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_LOAD_C_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_B, .reg1 = RT_C, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_C, .reg1 = RT_C, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_D, .reg1 = RT_C, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_E, .reg1 = RT_C, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_H, .reg1 = RT_C, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_L, .reg1 = RT_C, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_C_A, .reg1 = RT_C, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    
    [OP_LOAD_D_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_B, .reg1 = RT_D, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_C, .reg1 = RT_D, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_D, .reg1 = RT_D, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_E, .reg1 = RT_D, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_H, .reg1 = RT_D, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_L, .reg1 = RT_D, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_D_A, .reg1 = RT_D, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_LOAD_E_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_B, .reg1 = RT_E, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_C, .reg1 = RT_E, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_D, .reg1 = RT_E, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_E, .reg1 = RT_E, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_H, .reg1 = RT_E, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_L, .reg1 = RT_E, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_E_A, .reg1 = RT_E, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_LOAD_H_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_B, .reg1 = RT_H, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_C, .reg1 = RT_H, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_D, .reg1 = RT_H, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_E, .reg1 = RT_H, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_H, .reg1 = RT_H, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_L, .reg1 = RT_H, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_H_A, .reg1 = RT_H, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_LOAD_L_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_B, .reg1 = RT_L, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_C, .reg1 = RT_L, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_D, .reg1 = RT_L, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_E, .reg1 = RT_L, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_H, .reg1 = RT_L, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_L, .reg1 = RT_L, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_L_A, .reg1 = RT_L, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_LOAD_A_B] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_C] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_D] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_E] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_H] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_L] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_A] = { .handler = instrLoadRegReg, .mode = AM_REG_REG, .opcode = OP_LOAD_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 8 bit load immediate to reg 
    [OP_LOAD_B_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_B_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_C_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_D_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_E_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_H_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_L_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_8_BIT_IMM] = { .handler = instrLoadReg8bitImm, .opcode = OP_LOAD_A_8_BIT_IMM, .mode = AM_REG_8_BIT_IMM, .reg1 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 8 bit load from memory whose addr is at HL to a reg 
    [OP_LOAD_B_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_B_HL, .reg1 = RT_B, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_C_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_C_HL, .reg1 = RT_C, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_D_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_D_HL, .reg1 = RT_D, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_E_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_E_HL, .reg1 = RT_E, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_H_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_H_HL, .reg1 = RT_H, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_L_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_L_HL, .reg1 = RT_L, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_HL] = { .handler = instrLoadRegHL, .mode = AM_REG_MEM, .opcode = OP_LOAD_A_HL, .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 8 bit stores to memory whose addr is at HL from a reg
    [OP_LOAD_HL_B] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_B, .reg1 = RT_HL, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_C] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_C, .reg1 = RT_HL, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_D] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_D, .reg1 = RT_HL, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_E] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_E, .reg1 = RT_HL, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_H] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_H, .reg1 = RT_HL, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_L] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_L, .reg1 = RT_HL, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_A] = { .handler = instrLoadHLReg, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_A, .reg1 = RT_HL, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 8 bit store to memory whose addr is at HL from an immediate
    [OP_LOAD_HL_8_BIT_IMM] = { .handler = instrLoadHL8bitImm, .mode = AM_MEM_REG, .opcode = OP_LOAD_HL_8_BIT_IMM,.reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - load to and fro accumulator
    [OP_LOAD_BC_A] = { .handler = instrLoadReg16A, .mode = AM_MEM_REG, .opcode = OP_LOAD_BC_A, .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_DE_A] = { .handler = instrLoadReg16A, .mode = AM_MEM_REG, .opcode = OP_LOAD_DE_A, .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_BC] = { .handler = instrLoadAReg16, .mode = AM_REG_MEM, .opcode = OP_LOAD_A_BC, .reg2 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_DE] = { .handler = instrLoadAReg16, .mode = AM_REG_MEM, .opcode = OP_LOAD_A_DE, .reg2 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - load to addr at imm from accumulator 
    [OP_LOAD_16_BIT_IMM_A] = { .handler = instrLoad16BitImmA, .mode = AM_16_BIT_ADDR_REG, .opcode = OP_LOAD_16_BIT_IMM_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_16_BIT_IMM] = { .handler = instrLoadA16BitImm, .mode = AM_16_BIT_IMM,      .opcode = OP_LOAD_A_16_BIT_IMM, .reg1 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - load high ram, offset c, register is A 
    [OP_LOAD_HIGH_C_A] = { .handler = instrLoadHighCA, .mode = AM_COMP_REG_HIGH_RAM, .opcode = OP_LOAD_HIGH_C_A, .reg1 = RT_C, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HIGH_A_C] = { .handler = instrLoadHighAC, .mode = AM_HIGH_RAM_COMP_REG, .opcode = OP_LOAD_HIGH_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - load high ram, immediate, A 
    [OP_LOAD_HIGH_8_BIT_IMM_A] = { .handler = instrLoadHigh8BitImmA, .opcode = OP_LOAD_HIGH_8_BIT_IMM_A, .mode = AM_ADDR_HIGH_RAM, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HIGH_A_8_BIT_IMM] = { .handler = instrLoadHighA8BitImm, .opcode = OP_LOAD_HIGH_A_8_BIT_IMM, .mode = AM_HIGH_RAM_ADDR, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - increment, decrement loads 
    [OP_LOAD_HL_INCR_A] = { .handler = instrLoadHLIncDecA, .mode = AM_HL_INCR_REG, .opcode = OP_LOAD_HL_INCR_A, .reg1 = RT_HL, .reg2 = RT_A,  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_HL_INCR] = { .handler = instrLoadAHLIncDec, .mode = AM_REG_HL_INCR, .opcode = OP_LOAD_A_HL_INCR, .reg1 = RT_A,  .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_DECR_A] = { .handler = instrLoadHLIncDecA, .mode = AM_HL_DECR_REG, .opcode = OP_LOAD_HL_DECR_A, .reg1 = RT_HL, .reg2 = RT_A,  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_A_HL_DECR] = { .handler = instrLoadAHLIncDec, .mode = AM_REG_HL_DECR, .opcode = OP_LOAD_A_HL_DECR, .reg1 = RT_A,  .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 16 bit reg immediate loads 
    [OP_LOAD_BC_16_BIT_IMM] = { .handler = instrLoad16BitReg16BitImm, .mode = AM_REG_16_BIT_IMM, .opcode = OP_LOAD_BC_16_BIT_IMM, .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_DE_16_BIT_IMM] = { .handler = instrLoad16BitReg16BitImm, .mode = AM_REG_16_BIT_IMM, .opcode = OP_LOAD_DE_16_BIT_IMM, .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_HL_16_BIT_IMM] = { .handler = instrLoad16BitReg16BitImm, .mode = AM_REG_16_BIT_IMM, .opcode = OP_LOAD_HL_16_BIT_IMM, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_LOAD_SP_16_BIT_IMM] = { .handler = instrLoad16BitReg16BitImm, .mode = AM_REG_16_BIT_IMM, .opcode = OP_LOAD_SP_16_BIT_IMM, .reg1 = RT_SP, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 16 bit load immediate sp 
    [OP_LOAD_16_BIT_IMM_SP] = { .handler = instrLoad16BitImmSP, .mode = AM_16_BIT_ADDR_REG, .opcode = OP_LOAD_16_BIT_IMM_SP, .reg2 = RT_SP, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - load sp, hl 
    [OP_LOAD_SP_HL] = { .handler = instrLoadSpHl, .mode = AM_REG_REG, .opcode = OP_LOAD_SP_HL, .reg1 = RT_SP, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Push and Pop 
    [OP_PUSH_BC] = { .handler = instrPush, .mode = AM_REG, .opcode = OP_PUSH_BC, .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_PUSH_DE] = { .handler = instrPush, .mode = AM_REG, .opcode = OP_PUSH_DE, .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_PUSH_HL] = { .handler = instrPush, .mode = AM_REG, .opcode = OP_PUSH_HL, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_PUSH_AF] = { .handler = instrPush, .mode = AM_REG, .opcode = OP_PUSH_AF, .reg1 = RT_AF, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_POP_BC]  = { .handler = instrPop,  .mode = AM_REG, .opcode = OP_POP_BC,  .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_POP_DE]  = { .handler = instrPop,  .mode = AM_REG, .opcode = OP_POP_DE,  .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_POP_HL]  = { .handler = instrPop,  .mode = AM_REG, .opcode = OP_POP_HL,  .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_POP_AF]  = { .handler = instrPop,  .mode = AM_REG, .opcode = OP_POP_AF,  .reg1 = RT_AF, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Load with offset 
    [OP_LOAD_HL_SP_E8] = { .handler = instrLoadHlSpE8, .mode = AM_STACK_PTR_SIGNED_OFFSET, .opcode = OP_LOAD_HL_SP_E8, .reg1 = RT_HL, .reg2 = RT_SP, .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - 8 bit register add 
    [OP_ADD_A_B] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_C] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_D] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_E] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_H] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_L] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_A] = { .handler = instrAddAReg, .mode = AM_REG_REG, .opcode = OP_ADD_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - Indirect A adds 
    [OP_ADD_A_HL]        = { .handler = instrAddAHL,      .mode = AM_REG_MEM,       .opcode = OP_ADD_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_A_8_BIT_IMM] = { .handler = instrAddA8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_ADD_A_8_BIT_IMM, .reg1 = RT_A,                .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - ADC reg 
    [OP_ADC_A_B] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_C] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_D] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_E] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_H] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_L] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_A] = { .handler = instrAdcReg, .mode = AM_REG_REG, .opcode = OP_ADC_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - Indirect ADC 
    [OP_ADC_A_8_BIT_IMM] = { .handler = instrAdc8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_ADC_A_8_BIT_IMM, .reg1 = RT_A,                .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADC_A_HL]        = { .handler = instrAdcHL,      .mode = AM_REG_REG,       .opcode = OP_ADC_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - 8 bit register sub
    [OP_SUB_A_B] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_C] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_D] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_E] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_H] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_L] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_A] = { .handler = instrSubReg, .mode = AM_REG_REG, .opcode = OP_SUB_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - indirect Sub 
    [OP_SUB_A_HL]         = { .handler = instrSubHL,      .mode = AM_REG_MEM,       .opcode = OP_SUB_A_HL,        .reg1 = RT_A, .reg2 = RT_HL,  .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SUB_A_8_BIT_IMM]  = { .handler = instrSub8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_SUB_A_8_BIT_IMM, .reg1 = RT_A,                 .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - SBC reg 
    [OP_SBC_A_B] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_C] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_D] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_E] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_H] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_L] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_A] = { .handler = instrSbcReg, .mode = AM_REG_REG, .opcode = OP_SBC_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - Indirect SBC 
    [OP_SBC_A_HL]         = { .handler = instrSbcHL,      .mode = AM_REG_MEM,       .opcode = OP_SBC_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_SBC_A_8_BIT_IMM]  = { .handler = instrSbc8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_SBC_A_8_BIT_IMM, .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - Compare with reg 
    [OP_COMP_A_B] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_C] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_D] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_E] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_H] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_L] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_A] = { .handler = instrCompareReg, .mode = AM_REG_REG, .opcode = OP_COMP_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - indirect compares 
    [OP_COMP_A_HL]        = { .handler = instrCompareHL,      .mode = AM_REG_MEM,       .opcode = OP_COMP_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },
    [OP_COMP_A_8_BIT_IMM] = { .handler = instrCompare8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_COMP_A_8_BIT_IMM, .reg1 = RT_A,                .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_DEPENDS) },

    // - - - 8 bit increments 
    [OP_INC_B] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_B, .reg1 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_C] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_C, .reg1 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_D] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_D, .reg1 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_E] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_E, .reg1 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_H] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_H, .reg1 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_L] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_L, .reg1 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_INC_A] = { .handler = instrIncrementReg, .mode = AM_REG, .opcode = OP_INC_A, .reg1 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    
    // - - - 8 bit derecment  
    [OP_DEC_B] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_B, .reg1 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_C] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_C, .reg1 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_D] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_D, .reg1 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_E] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_E, .reg1 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_H] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_H, .reg1 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_L] = { .handler = instrDecrementReg, .mode = AM_REG, .opcode = OP_DEC_L, .reg1 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_A] = { .handler = instrDecrementReg, .mode = AM_REG, .reg1 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },

    // - - - Indirect incr, decr 
    [OP_INC_HL] = { .handler = instrIncrementHL, .mode = AM_REG_MEM, .opcode = OP_INC_HL, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_DEPENDS, FB_KEEP) },
    [OP_DEC_HL] = { .handler = instrDecrementHL, .mode = AM_REG_MEM, .opcode = OP_DEC_HL, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_SET, FB_DEPENDS, FB_KEEP) },

    // - - - 8 bit AND 
    [OP_AND_A_B] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_C] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_D] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_E] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_H] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_L] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_A] = { .handler = instrAndReg, .mode = AM_REG_REG, .opcode = OP_AND_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },

    // - - - Indirect AND 
    [OP_AND_A_HL]         = { .handler = instrAndHL,      .mode = AM_REG_MEM,       .opcode = OP_AND_A_HL,        .reg1 = RT_A, .reg2 = RT_HL,  .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },
    [OP_AND_A_8_BIT_IMM]  = { .handler = instrAnd8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_AND_A_8_BIT_IMM, .reg1 = RT_A,                 .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_RESET) },

    // - - - 8 bit OR 
    [OP_OR_A_B] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_C] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_D] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_E] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_H] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_L] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_A] = { .handler = instrOrReg, .mode = AM_REG_REG, .opcode = OP_OR_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    // - - - Indirect ORs 
    [OP_OR_A_HL]        = { .handler = instrOrHL,      .mode = AM_REG_MEM,       .opcode = OP_OR_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_OR_A_8_BIT_IMM] = { .handler = instrOr8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_OR_A_8_BIT_IMM, .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    // - - - 8 bit XOR 
    [OP_XOR_A_B] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_B, .reg1 = RT_A, .reg2 = RT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_C] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_C, .reg1 = RT_A, .reg2 = RT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_D] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_D, .reg1 = RT_A, .reg2 = RT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_E] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_E, .reg1 = RT_A, .reg2 = RT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_H] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_H, .reg1 = RT_A, .reg2 = RT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_L] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_L, .reg1 = RT_A, .reg2 = RT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_A] = { .handler = instrXorReg, .mode = AM_REG_REG, .opcode = OP_XOR_A_A, .reg1 = RT_A, .reg2 = RT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    // - - - indirect XOR 
    [OP_XOR_A_8_BIT_IMM]  = { .handler = instrXor8BitImm, .mode = AM_REG_8_BIT_IMM, .opcode = OP_XOR_A_8_BIT_IMM, .reg1 = RT_A,                .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_XOR_A_HL]         = { .handler = instrXorHL,      .mode = AM_REG_MEM,       .opcode = OP_XOR_A_HL,        .reg1 = RT_A, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    // - - - extra arithmetic
    [OP_CCF] = { .handler = instrCcf, .mode = AM_IMPLIED, .opcode = OP_CCF, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_SCF] = { .handler = instrScf, .mode = AM_IMPLIED, .opcode = OP_SCF, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_RESET, FB_SET) },
    [OP_DAA] = { .handler = instrDaa, .mode = AM_IMPLIED, .opcode = OP_DAA, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_KEEP, FB_RESET, FB_DEPENDS) },
    [OP_CPL] = { .handler = instrCpl, .mode = AM_IMPLIED, .opcode = OP_CPL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_SET, FB_SET, FB_KEEP) },

    // - - - 16 bit increment 
    [OP_INC_BC]     = { .handler = instrIncrementRegReg, .mode = AM_REG, .opcode = OP_INC_BC,     .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_INC_DE]     = { .handler = instrIncrementRegReg, .mode = AM_REG, .opcode = OP_INC_DE,     .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_INC_HL_REG] = { .handler = instrIncrementRegReg, .mode = AM_REG, .opcode = OP_INC_HL_REG, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_INC_SP]     = { .handler = instrIncrementRegReg, .mode = AM_REG, .opcode = OP_INC_SP,     .reg1 = RT_SP, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 16 bit decrement 
    [OP_DEC_BC]     = { .handler = instrDecrementRegReg, .mode = AM_REG, .opcode = OP_DEC_BC,     .reg1 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_DEC_DE]     = { .handler = instrDecrementRegReg, .mode = AM_REG, .opcode = OP_DEC_DE,     .reg1 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_DEC_HL_REG] = { .handler = instrDecrementRegReg, .mode = AM_REG, .opcode = OP_DEC_HL_REG, .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_DEC_SP]     = { .handler = instrDecrementRegReg, .mode = AM_REG, .opcode = OP_DEC_SP,     .reg1 = RT_SP, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - 16 bit ADD 
    [OP_ADD_HL_BC] = { .handler = instrAddHlRegReg, .mode = AM_REG_REG,                 .opcode = OP_ADD_HL_BC, .reg1 = RT_HL, .reg2 = RT_BC, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_HL_DE] = { .handler = instrAddHlRegReg, .mode = AM_REG_REG,                 .opcode = OP_ADD_HL_DE, .reg1 = RT_HL, .reg2 = RT_DE, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_HL_HL] = { .handler = instrAddHlRegReg, .mode = AM_REG_REG,                 .opcode = OP_ADD_HL_HL, .reg1 = RT_HL, .reg2 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_HL_SP] = { .handler = instrAddHlRegReg, .mode = AM_REG_REG,                 .opcode = OP_ADD_HL_SP, .reg1 = RT_HL, .reg2 = RT_SP, .flags = FLAGPACK_MAKE(FB_KEEP, FB_RESET, FB_DEPENDS, FB_DEPENDS) },
    [OP_ADD_SP_E8] = { .handler = instrAddSpE8,     .mode = AM_STACK_PTR_SIGNED_OFFSET, .opcode = OP_ADD_SP_E8, .reg1 = RT_SP,                .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_DEPENDS, FB_DEPENDS) },

    // - - - Shifts 
    [OP_ROTATE_LEFT_CIRCULAR_A]  = { .handler = instrRlca, .mode = AM_IMPLIED, .opcode = OP_ROTATE_LEFT_CIRCULAR_A,  .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_ROTATE_RIGHT_CIRCULAR_A] = { .handler = instrRrca, .mode = AM_IMPLIED, .opcode = OP_ROTATE_RIGHT_CIRCULAR_A, .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_ROTATE_LEFT_A]           = { .handler = instrRla,  .mode = AM_IMPLIED, .opcode = OP_ROTATE_LEFT_A,           .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_ROTATE_RIGHT_A]          = { .handler = instrRra,  .mode = AM_IMPLIED, .opcode = OP_ROTATE_RIGHT_A,          .flags = FLAGPACK_MAKE(FB_RESET, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - JUmps
    [OP_JUMP_16_BIT_IMM]           = { .handler = instrJump16BitImm,                    .mode = AM_16_BIT_IMM, .opcode = OP_JUMP_16_BIT_IMM,                   .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_HL]                   = { .handler = instrJumpHL,                          .mode = AM_REG,        .opcode = OP_JUMP_HL,            .reg1 = RT_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_NZ_16_BIT_IMM]        = { .handler = instrJumpConditional16BitImm,         .mode = AM_16_BIT_IMM, .opcode = OP_JUMP_NZ_16_BIT_IMM,                .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_Z_16_BIT_IMM]         = { .handler = instrJumpConditional16BitImm,         .mode = AM_16_BIT_IMM, .opcode = OP_JUMP_Z_16_BIT_IMM,                 .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_NC_16_BIT_IMM]        = { .handler = instrJumpConditional16BitImm,         .mode = AM_16_BIT_IMM, .opcode = OP_JUMP_NC_16_BIT_IMM,                .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_C_16_BIT_IMM]         = { .handler = instrJumpConditional16BitImm,         .mode = AM_16_BIT_IMM, .opcode = OP_JUMP_C_16_BIT_IMM,                 .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_SIGNED_8_BIT_IMM]     = { .handler = instrJumpRelSigned8BitImm,            .mode = AM_8_BIT_IMM,  .opcode = OP_JUMP_SIGNED_8_BIT_IMM,             .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_NZ_SIGNED_8_BIT_IMM]  = { .handler = instrJumpRelConditionalSigned8BitImm, .mode = AM_8_BIT_IMM,  .opcode = OP_JUMP_NZ_SIGNED_8_BIT_IMM,          .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_Z_SIGNED_8_BIT_IMM]   = { .handler = instrJumpRelConditionalSigned8BitImm, .mode = AM_8_BIT_IMM,  .opcode = OP_JUMP_Z_SIGNED_8_BIT_IMM,           .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_NC_SIGNED_8_BIT_IMM]  = { .handler = instrJumpRelConditionalSigned8BitImm, .mode = AM_8_BIT_IMM,  .opcode = OP_JUMP_NC_SIGNED_8_BIT_IMM,          .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_JUMP_C_SIGNED_8_BIT_IMM]   = { .handler = instrJumpRelConditionalSigned8BitImm, .mode = AM_8_BIT_IMM,  .opcode = OP_JUMP_C_SIGNED_8_BIT_IMM,           .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Call 
    [OP_CALL_16_BIT_IMM]    = { .handler = instrCall16BitImm,            .mode = AM_16_BIT_IMM, .opcode = OP_CALL_16_BIT_IMM,    .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CALL_NZ_16_BIT_IMM] = { .handler = instrCallConditional16BitImm, .mode = AM_16_BIT_IMM, .opcode = OP_CALL_NZ_16_BIT_IMM, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CALL_Z_16_BIT_IMM]  = { .handler = instrCallConditional16BitImm, .mode = AM_16_BIT_IMM, .opcode = OP_CALL_Z_16_BIT_IMM,  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CALL_NC_16_BIT_IMM] = { .handler = instrCallConditional16BitImm, .mode = AM_16_BIT_IMM, .opcode = OP_CALL_NC_16_BIT_IMM, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CALL_C_16_BIT_IMM]  = { .handler = instrCallConditional16BitImm, .mode = AM_16_BIT_IMM, .opcode = OP_CALL_C_16_BIT_IMM,  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Return 
    [OP_RETURN]           = { .handler = instrReturn,                  .mode = AM_IMPLIED, .opcode = OP_RETURN,           .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RETURN_NZ]        = { .handler = instrCallConditional16BitImm, .mode = AM_IMPLIED, .opcode = OP_RETURN_NZ,        .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RETURN_Z]         = { .handler = instrCallConditional16BitImm, .mode = AM_IMPLIED, .opcode = OP_RETURN_Z,         .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RETURN_NC]        = { .handler = instrCallConditional16BitImm, .mode = AM_IMPLIED, .opcode = OP_RETURN_NC,        .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RETURN_C]         = { .handler = instrCallConditional16BitImm, .mode = AM_IMPLIED, .opcode = OP_RETURN_C,         .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RETURN_INTERRUPT] = { .handler = instrReturnInterrupt,         .mode = AM_IMPLIED, .opcode = OP_RETURN_INTERRUPT, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_RESTART_00] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_00, .param = 0x00, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_08] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_08, .param = 0x08, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_10] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_10, .param = 0x10, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_18] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_18, .param = 0x18, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_20] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_20, .param = 0x20, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_28] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_28, .param = 0x28, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_30] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_30, .param = 0x30, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_RESTART_38] = { .handler = instrRestart, .mode = AM_IMPLIED, .opcode = OP_RESTART_38, .param = 0x38, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Misc 
    [OP_DISABLE_INTERRUPT]  = { .handler = instrDisableInterrupt, .mode = AM_IMPLIED, .opcode = OP_DISABLE_INTERRUPT, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_ENABLE_INTERRUPT]   = { .handler = instrEnableInterrupt,  .mode = AM_IMPLIED, .opcode = OP_ENABLE_INTERRUPT,  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_NOP]                = { .handler = instrNop,              .mode = AM_IMPLIED, .opcode = OP_NOP,               .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_STOP]               = { .handler = instrStop,             .mode = AM_IMPLIED, .opcode = OP_STOP,              .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_HALT]               = { .handler = instrHalt,             .mode = AM_IMPLIED, .opcode = OP_HALT,              .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [203] = { .handler = instrUnimplemented },
    [211] = { .handler = instrUnimplemented },
    [219] = { .handler = instrUnimplemented },
    [221] = { .handler = instrUnimplemented },
    [227] = { .handler = instrUnimplemented },
    [228] = { .handler = instrUnimplemented },
    [235] = { .handler = instrUnimplemented },
    [236] = { .handler = instrUnimplemented },
    [237] = { .handler = instrUnimplemented },
    [244] = { .handler = instrUnimplemented },
    [252] = { .handler = instrUnimplemented },
    [253] = { .handler = instrUnimplemented },
  };

static Instruction cbTable[INSTRUCTION_COUNT] = 
  {
    // - - - rotate left circular 
    [OP_CB_ROTATE_LEFT_CIRCULAR_B - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_C - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_D - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_E - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_H - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_L - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_CIRCULAR_A - CB_OFFSET] = { .handler = instrCbRlcReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_ROTATE_LEFT_CIRCULAR_HL - CB_OFFSET] = { .handler = instrCbRlcHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_ROTATE_LEFT_CIRCULAR_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - rotate right circular 
    [OP_CB_ROTATE_RIGHT_CIRCULAR_B - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_C - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_D - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_E - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_H - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_L - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_CIRCULAR_A - CB_OFFSET] = { .handler = instrCbRrcReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_ROTATE_RIGHT_CIRCULAR_HL - CB_OFFSET] = { .handler = instrCbRrcHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_ROTATE_RIGHT_CIRCULAR_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - roate left 

    [OP_CB_ROTATE_LEFT_B - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_ROTATE_LEFT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_C - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_ROTATE_LEFT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_D - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_ROTATE_LEFT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_E - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_ROTATE_LEFT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_H - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_ROTATE_LEFT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_L - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_ROTATE_LEFT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_LEFT_A - CB_OFFSET] = { .handler = instrCbRlReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_ROTATE_LEFT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_ROTATE_LEFT_HL - CB_OFFSET] = { .handler = instrCbRlHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_ROTATE_LEFT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - rotate right  
    [OP_CB_ROTATE_RIGHT_B - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_ROTATE_RIGHT_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_C - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_ROTATE_RIGHT_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_D - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_ROTATE_RIGHT_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_E - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_ROTATE_RIGHT_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_H - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_ROTATE_RIGHT_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_L - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_ROTATE_RIGHT_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_ROTATE_RIGHT_A - CB_OFFSET] = { .handler = instrCbRrReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_ROTATE_RIGHT_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_ROTATE_RIGHT_HL - CB_OFFSET] = { .handler = instrCbRrHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_ROTATE_RIGHT_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - shift left arithmetic 
    [OP_CB_SHIFT_LEFT_ARITH_B - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_SHIFT_LEFT_ARITH_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_C - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_SHIFT_LEFT_ARITH_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_D - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_SHIFT_LEFT_ARITH_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_E - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_SHIFT_LEFT_ARITH_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_H - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_SHIFT_LEFT_ARITH_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_L - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_SHIFT_LEFT_ARITH_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_LEFT_ARITH_A - CB_OFFSET] = { .handler = instrCbSlaReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_SHIFT_LEFT_ARITH_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_SHIFT_LEFT_ARITH_HL - CB_OFFSET] = { .handler = instrCbSlaHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_SHIFT_LEFT_ARITH_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - shift right arithmetic 
    [OP_CB_SHIFT_RIGHT_ARITH_B - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_SHIFT_RIGHT_ARITH_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_C - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_SHIFT_RIGHT_ARITH_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_D - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_SHIFT_RIGHT_ARITH_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_E - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_SHIFT_RIGHT_ARITH_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_H - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_SHIFT_RIGHT_ARITH_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_L - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_SHIFT_RIGHT_ARITH_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_ARITH_A - CB_OFFSET] = { .handler = instrCbSraReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_SHIFT_RIGHT_ARITH_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    
    [OP_CB_SHIFT_RIGHT_ARITH_HL - CB_OFFSET] = { .handler = instrCbSraHL,  .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_SHIFT_RIGHT_ARITH_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - SWAP 
    [OP_CB_SWAP_B - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_SWAP_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_C - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_SWAP_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_D - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_SWAP_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_E - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_SWAP_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_H - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_SWAP_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_L - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_SWAP_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },
    [OP_CB_SWAP_A - CB_OFFSET] = { .handler = instrCbSwapReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_SWAP_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    [OP_CB_SWAP_HL - CB_OFFSET] = { .handler = instrCbSwapHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_SWAP_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_RESET) },

    // - - - shift right logical 
    [OP_CB_SHIFT_RIGHT_LOGIC_B - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_B, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_C - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_C, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_D - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_D, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_E - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_E, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_H - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_H, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_L - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_L, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },
    [OP_CB_SHIFT_RIGHT_LOGIC_A - CB_OFFSET] = { .handler = instrCbSrlReg, .mode = AM_REG, .reg1 = RT_A, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    [OP_CB_SHIFT_RIGHT_LOGIC_HL - CB_OFFSET] = { .handler = instrCbSrlHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .opcode = OP_CB_SHIFT_RIGHT_LOGIC_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_RESET, FB_DEPENDS) },

    // - - - Bit read 
    [OP_CB_BIT_0_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 0, .opcode = OP_CB_BIT_0_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 0, .opcode = OP_CB_BIT_0_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 0, .opcode = OP_CB_BIT_0_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 0, .opcode = OP_CB_BIT_0_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 0, .opcode = OP_CB_BIT_0_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 0, .opcode = OP_CB_BIT_0_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_0_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 0, .opcode = OP_CB_BIT_0_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_0_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 0, .opcode = OP_CB_BIT_0_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    
    [OP_CB_BIT_1_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 1, .opcode = OP_CB_BIT_1_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 1, .opcode = OP_CB_BIT_1_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 1, .opcode = OP_CB_BIT_1_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 1, .opcode = OP_CB_BIT_1_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 1, .opcode = OP_CB_BIT_1_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 1, .opcode = OP_CB_BIT_1_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_1_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 1, .opcode = OP_CB_BIT_1_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_1_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 1, .opcode = OP_CB_BIT_1_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_2_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 2, .opcode = OP_CB_BIT_2_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 2, .opcode = OP_CB_BIT_2_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 2, .opcode = OP_CB_BIT_2_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 2, .opcode = OP_CB_BIT_2_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 2, .opcode = OP_CB_BIT_2_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 2, .opcode = OP_CB_BIT_2_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_2_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 2, .opcode = OP_CB_BIT_2_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_2_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 2, .opcode = OP_CB_BIT_2_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_3_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 3, .opcode = OP_CB_BIT_3_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 3, .opcode = OP_CB_BIT_3_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 3, .opcode = OP_CB_BIT_3_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 3, .opcode = OP_CB_BIT_3_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 3, .opcode = OP_CB_BIT_3_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 3, .opcode = OP_CB_BIT_3_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_3_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 3, .opcode = OP_CB_BIT_3_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_3_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 3, .opcode = OP_CB_BIT_3_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_4_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 4, .opcode = OP_CB_BIT_4_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 4, .opcode = OP_CB_BIT_4_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 4, .opcode = OP_CB_BIT_4_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 4, .opcode = OP_CB_BIT_4_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 4, .opcode = OP_CB_BIT_4_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 4, .opcode = OP_CB_BIT_4_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_4_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 4, .opcode = OP_CB_BIT_4_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_4_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 4, .opcode = OP_CB_BIT_4_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_5_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 5, .opcode = OP_CB_BIT_5_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 5, .opcode = OP_CB_BIT_5_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 5, .opcode = OP_CB_BIT_5_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 5, .opcode = OP_CB_BIT_5_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 5, .opcode = OP_CB_BIT_5_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 5, .opcode = OP_CB_BIT_5_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_5_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 5, .opcode = OP_CB_BIT_5_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_5_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 5, .opcode = OP_CB_BIT_5_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_6_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 6, .opcode = OP_CB_BIT_6_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 6, .opcode = OP_CB_BIT_6_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 6, .opcode = OP_CB_BIT_6_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 6, .opcode = OP_CB_BIT_6_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 6, .opcode = OP_CB_BIT_6_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 6, .opcode = OP_CB_BIT_6_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_6_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 6, .opcode = OP_CB_BIT_6_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_6_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 6, .opcode = OP_CB_BIT_6_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_7_B - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_B, .param = 7, .opcode = OP_CB_BIT_7_B, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_C - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_C, .param = 7, .opcode = OP_CB_BIT_7_C, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_D - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_D, .param = 7, .opcode = OP_CB_BIT_7_D, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_E - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_E, .param = 7, .opcode = OP_CB_BIT_7_E, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_H - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_H, .param = 7, .opcode = OP_CB_BIT_7_H, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_L - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_L, .param = 7, .opcode = OP_CB_BIT_7_L, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },
    [OP_CB_BIT_7_A - CB_OFFSET] = { .handler = instrCbBitReg, .mode = AM_REG, .reg1 = RT_A, .param = 7, .opcode = OP_CB_BIT_7_A, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    [OP_CB_BIT_7_HL - CB_OFFSET] = { .handler = instrCbBitHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 7, .opcode = OP_CB_BIT_7_HL, .flags = FLAGPACK_MAKE(FB_DEPENDS, FB_RESET, FB_SET, FB_KEEP) },

    // - - - Bit reset
    [OP_CB_RESET_0_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 0, .opcode = OP_CB_RESET_0_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 0, .opcode = OP_CB_RESET_0_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 0, .opcode = OP_CB_RESET_0_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 0, .opcode = OP_CB_RESET_0_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 0, .opcode = OP_CB_RESET_0_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 0, .opcode = OP_CB_RESET_0_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_0_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 0, .opcode = OP_CB_RESET_0_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_0_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 0, .opcode = OP_CB_RESET_0_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    
    [OP_CB_RESET_1_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 1, .opcode = OP_CB_RESET_1_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 1, .opcode = OP_CB_RESET_1_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 1, .opcode = OP_CB_RESET_1_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 1, .opcode = OP_CB_RESET_1_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 1, .opcode = OP_CB_RESET_1_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 1, .opcode = OP_CB_RESET_1_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_1_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 1, .opcode = OP_CB_RESET_1_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_1_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 1, .opcode = OP_CB_RESET_1_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_2_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 2, .opcode = OP_CB_RESET_2_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 2, .opcode = OP_CB_RESET_2_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 2, .opcode = OP_CB_RESET_2_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 2, .opcode = OP_CB_RESET_2_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 2, .opcode = OP_CB_RESET_2_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 2, .opcode = OP_CB_RESET_2_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_2_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 2, .opcode = OP_CB_RESET_2_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_2_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 2, .opcode = OP_CB_RESET_2_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_3_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 3, .opcode = OP_CB_RESET_3_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 3, .opcode = OP_CB_RESET_3_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 3, .opcode = OP_CB_RESET_3_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 3, .opcode = OP_CB_RESET_3_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 3, .opcode = OP_CB_RESET_3_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 3, .opcode = OP_CB_RESET_3_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_3_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 3, .opcode = OP_CB_RESET_3_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_3_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 3, .opcode = OP_CB_RESET_3_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_4_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 4, .opcode = OP_CB_RESET_4_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 4, .opcode = OP_CB_RESET_4_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 4, .opcode = OP_CB_RESET_4_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 4, .opcode = OP_CB_RESET_4_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 4, .opcode = OP_CB_RESET_4_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 4, .opcode = OP_CB_RESET_4_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_4_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 4, .opcode = OP_CB_RESET_4_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_4_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 4, .opcode = OP_CB_RESET_4_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_5_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 5, .opcode = OP_CB_RESET_5_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 5, .opcode = OP_CB_RESET_5_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 5, .opcode = OP_CB_RESET_5_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 5, .opcode = OP_CB_RESET_5_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 5, .opcode = OP_CB_RESET_5_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 5, .opcode = OP_CB_RESET_5_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_5_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 5, .opcode = OP_CB_RESET_5_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_5_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 5, .opcode = OP_CB_RESET_5_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_6_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 6, .opcode = OP_CB_RESET_6_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 6, .opcode = OP_CB_RESET_6_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 6, .opcode = OP_CB_RESET_6_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 6, .opcode = OP_CB_RESET_6_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 6, .opcode = OP_CB_RESET_6_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 6, .opcode = OP_CB_RESET_6_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_6_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 6, .opcode = OP_CB_RESET_6_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_6_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 6, .opcode = OP_CB_RESET_6_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_7_B - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_B, .param = 7, .opcode = OP_CB_RESET_7_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_C - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_C, .param = 7, .opcode = OP_CB_RESET_7_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_D - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_D, .param = 7, .opcode = OP_CB_RESET_7_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_E - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_E, .param = 7, .opcode = OP_CB_RESET_7_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_H - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_H, .param = 7, .opcode = OP_CB_RESET_7_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_L - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_L, .param = 7, .opcode = OP_CB_RESET_7_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_RESET_7_A - CB_OFFSET] = { .handler = instrCbResReg, .mode = AM_REG, .reg1 = RT_A, .param = 7, .opcode = OP_CB_RESET_7_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_RESET_7_HL - CB_OFFSET] = { .handler = instrCbResHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 7, .opcode = OP_CB_RESET_7_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    // - - - Bit ret
    [OP_CB_SET_0_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 0, .opcode = OP_CB_SET_0_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 0, .opcode = OP_CB_SET_0_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 0, .opcode = OP_CB_SET_0_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 0, .opcode = OP_CB_SET_0_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 0, .opcode = OP_CB_SET_0_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 0, .opcode = OP_CB_SET_0_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_0_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 0, .opcode = OP_CB_SET_0_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_0_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 0, .opcode = OP_CB_SET_0_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    
    [OP_CB_SET_1_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 1, .opcode = OP_CB_SET_1_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 1, .opcode = OP_CB_SET_1_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 1, .opcode = OP_CB_SET_1_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 1, .opcode = OP_CB_SET_1_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 1, .opcode = OP_CB_SET_1_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 1, .opcode = OP_CB_SET_1_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_1_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 1, .opcode = OP_CB_SET_1_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_1_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 1, .opcode = OP_CB_SET_1_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_2_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 2, .opcode = OP_CB_SET_2_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 2, .opcode = OP_CB_SET_2_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 2, .opcode = OP_CB_SET_2_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 2, .opcode = OP_CB_SET_2_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 2, .opcode = OP_CB_SET_2_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 2, .opcode = OP_CB_SET_2_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_2_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 2, .opcode = OP_CB_SET_2_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_2_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 2, .opcode = OP_CB_SET_2_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_3_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 3, .opcode = OP_CB_SET_3_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 3, .opcode = OP_CB_SET_3_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 3, .opcode = OP_CB_SET_3_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 3, .opcode = OP_CB_SET_3_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 3, .opcode = OP_CB_SET_3_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 3, .opcode = OP_CB_SET_3_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_3_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 3, .opcode = OP_CB_SET_3_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_3_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 3, .opcode = OP_CB_SET_3_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_4_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 4, .opcode = OP_CB_SET_4_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 4, .opcode = OP_CB_SET_4_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 4, .opcode = OP_CB_SET_4_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 4, .opcode = OP_CB_SET_4_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 4, .opcode = OP_CB_SET_4_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 4, .opcode = OP_CB_SET_4_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_4_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 4, .opcode = OP_CB_SET_4_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_4_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 4, .opcode = OP_CB_SET_4_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_5_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 5, .opcode = OP_CB_SET_5_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 5, .opcode = OP_CB_SET_5_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 5, .opcode = OP_CB_SET_5_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 5, .opcode = OP_CB_SET_5_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 5, .opcode = OP_CB_SET_5_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 5, .opcode = OP_CB_SET_5_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_5_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 5, .opcode = OP_CB_SET_5_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_5_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 5, .opcode = OP_CB_SET_5_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_6_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 6, .opcode = OP_CB_SET_6_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 6, .opcode = OP_CB_SET_6_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 6, .opcode = OP_CB_SET_6_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 6, .opcode = OP_CB_SET_6_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 6, .opcode = OP_CB_SET_6_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 6, .opcode = OP_CB_SET_6_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_6_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 6, .opcode = OP_CB_SET_6_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_6_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 6, .opcode = OP_CB_SET_6_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_7_B - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_B, .param = 7, .opcode = OP_CB_SET_7_B, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_C - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_C, .param = 7, .opcode = OP_CB_SET_7_C, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_D - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_D, .param = 7, .opcode = OP_CB_SET_7_D, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_E - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_E, .param = 7, .opcode = OP_CB_SET_7_E, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_H - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_H, .param = 7, .opcode = OP_CB_SET_7_H, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_L - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_L, .param = 7, .opcode = OP_CB_SET_7_L, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
    [OP_CB_SET_7_A - CB_OFFSET] = { .handler = instrCbSetReg, .mode = AM_REG, .reg1 = RT_A, .param = 7, .opcode = OP_CB_SET_7_A, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },

    [OP_CB_SET_7_HL - CB_OFFSET] = { .handler = instrCbSetHL, .mode = AM_REG_MEM, .reg1 = RT_HL, .param = 7, .opcode = OP_CB_SET_7_HL, .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP) },
  };


const Instruction* instructionGetByOpcode(Opcode OPCODE)
{ return &opTable[(u8)OPCODE]; }

const Instruction* instructionGetByCBOpcode(Opcode OPCODE)
{ return &cbTable[(u8)(OPCODE - CB_OFFSET)]; }

/**
 * @brief Safety handler for unimplemented opcodes.
 * Triggers a fatal crash with diagnostic information.
*/
ExecStatus instrUnimplemented(void) 
{
  CpuContext* ctx = cpuGetContext();
  char        trace[128];
  cpuTraceLineToString(trace, sizeof(trace));
  
  FORGE_LOG_ERROR("FATAL: Unimplemented Opcode 0x%02X at PC 0x%04X", ctx->currentOpcode, ctx->pcAtFetch);
  FORGE_LOG_ERROR("Trace: %s", trace);
  
  TODO
  return EXEC_STATUS_DONE_IMMEDIATE; 
}

const char* instructionGetName(Opcode OPCODE)
{
  switch (OPCODE)
  {
    case OP_NOP             : return "NOP";
    case OP_CB_PREFIX       : return "CB Prefix";

    // - - - 8 bit register to register loads
    case OP_LOAD_B_B : return "LD B, B";
    case OP_LOAD_B_C : return "LD B, C";
    case OP_LOAD_B_D : return "LD B, D";
    case OP_LOAD_B_E : return "LD B, E";
    case OP_LOAD_B_H : return "LD B, H";
    case OP_LOAD_B_L : return "LD B, L";
    case OP_LOAD_B_A : return "LD B, A";
    case OP_LOAD_C_B : return "LD C, B";
    case OP_LOAD_C_C : return "LD C, C";
    case OP_LOAD_C_D : return "LD C, D";
    case OP_LOAD_C_E : return "LD C, E";
    case OP_LOAD_C_H : return "LD C, H";
    case OP_LOAD_C_L : return "LD C, L";
    case OP_LOAD_C_A : return "LD C, A";
    case OP_LOAD_D_B : return "LD D, B";
    case OP_LOAD_D_C : return "LD D, C";
    case OP_LOAD_D_D : return "LD D, D";
    case OP_LOAD_D_E : return "LD D, E";
    case OP_LOAD_D_H : return "LD D, H";
    case OP_LOAD_D_L : return "LD D, L";
    case OP_LOAD_D_A : return "LD D, A";
    case OP_LOAD_E_B : return "LD E, B";
    case OP_LOAD_E_C : return "LD E, C";
    case OP_LOAD_E_D : return "LD E, D";
    case OP_LOAD_E_E : return "LD E, E";
    case OP_LOAD_E_H : return "LD E, H";
    case OP_LOAD_E_L : return "LD E, L";
    case OP_LOAD_E_A : return "LD E, A";
    case OP_LOAD_H_B : return "LD H, B";
    case OP_LOAD_H_C : return "LD H, C";
    case OP_LOAD_H_D : return "LD H, D";
    case OP_LOAD_H_E : return "LD H, E";
    case OP_LOAD_H_H : return "LD H, H";
    case OP_LOAD_H_L : return "LD H, L";
    case OP_LOAD_H_A : return "LD H, A";
    case OP_LOAD_L_B : return "LD L, B";
    case OP_LOAD_L_C : return "LD L, C";
    case OP_LOAD_L_D : return "LD L, D";
    case OP_LOAD_L_E : return "LD L, E";
    case OP_LOAD_L_H : return "LD L, H";
    case OP_LOAD_L_L : return "LD L, L";
    case OP_LOAD_L_A : return "LD L, A";
    case OP_LOAD_A_B : return "LD A, B";
    case OP_LOAD_A_C : return "LD A, C";
    case OP_LOAD_A_D : return "LD A, D";
    case OP_LOAD_A_E : return "LD A, E";
    case OP_LOAD_A_H : return "LD A, H";
    case OP_LOAD_A_L : return "LD A, L";
    case OP_LOAD_A_A : return "LD A, A";

    // - - - 8 bit load imm to reg 
    case OP_LOAD_B_8_BIT_IMM : return "LD B, n8";
    case OP_LOAD_C_8_BIT_IMM : return "LD C, n8";
    case OP_LOAD_D_8_BIT_IMM : return "LD D, n8";
    case OP_LOAD_E_8_BIT_IMM : return "LD E, n8";
    case OP_LOAD_H_8_BIT_IMM : return "LD H, n8";
    case OP_LOAD_L_8_BIT_IMM : return "LD L, n8";
    case OP_LOAD_A_8_BIT_IMM : return "LD A, n8";

    // - - - 8 bit indirect loads from HL
    case OP_LOAD_B_HL : return "LD B, (HL)";
    case OP_LOAD_C_HL : return "LD C, (HL)";
    case OP_LOAD_D_HL : return "LD D, (HL)";
    case OP_LOAD_E_HL : return "LD E, (HL)";
    case OP_LOAD_H_HL : return "LD H, (HL)";
    case OP_LOAD_L_HL : return "LD L, (HL)";
    case OP_LOAD_A_HL : return "LD A, (HL)";

    // - - - 8 bit stores to HL
    case OP_LOAD_HL_B : return "LD (HL), B";
    case OP_LOAD_HL_C : return "LD (HL), C";
    case OP_LOAD_HL_D : return "LD (HL), D";
    case OP_LOAD_HL_E : return "LD (HL), E";
    case OP_LOAD_HL_H : return "LD (HL), H";
    case OP_LOAD_HL_L : return "LD (HL), L";
    case OP_LOAD_HL_A : return "LD (HL), A";

    // - - - 8 bit store to HL from immediate 
    case OP_LOAD_HL_8_BIT_IMM : return "LD (HL), n8";

    // - - - load to and from accumulator
    case OP_LOAD_A_BC : return "LD A, (BC)";
    case OP_LOAD_BC_A : return "LD (BC), A";
    case OP_LOAD_DE_A : return "LD (DE), A";
    case OP_LOAD_A_DE : return "LD A, (DE)";

    // - - - load to the address at imm from accumulator
    case OP_LOAD_16_BIT_IMM_A : return "LD (nn), A";
    case OP_LOAD_A_16_BIT_IMM : return "LD A, (nn)";

    // - - - high ram load from offset at C to A and vice versa
    case OP_LOAD_HIGH_C_A : return "LDH A, (C)";
    case OP_LOAD_HIGH_A_C : return "LDH (C), A";

    // - - - high ram load between a and 8 bit imm 
    case OP_LOAD_HIGH_8_BIT_IMM_A : return "LDH (n8), A";
    case OP_LOAD_HIGH_A_8_BIT_IMM : return "LDH A, (n8)";

    // - - - Increment Decrement oads 
    case OP_LOAD_HL_INCR_A : return "LD (HL+), A";
    case OP_LOAD_A_HL_INCR : return "LD A, (HL+)";
    case OP_LOAD_HL_DECR_A : return "LD (HL-), A";
    case OP_LOAD_A_HL_DECR : return "LD A, (HL-)";

    // - - - 16 bit reg immediate loads 
    case OP_LOAD_BC_16_BIT_IMM : return "LD BC, nn";
    case OP_LOAD_DE_16_BIT_IMM : return "LD DE, nn";
    case OP_LOAD_HL_16_BIT_IMM : return "LD HL, nn";
    case OP_LOAD_SP_16_BIT_IMM : return "LD SP, nn";

    // - - - Indirect loads 
    case OP_LOAD_16_BIT_IMM_SP  : return "LD (nn), SP";
    case OP_LOAD_SP_HL          : return "LD SP, HL";

    // - - - Push and Pop
    case OP_PUSH_BC : return "PUSH BC";
    case OP_PUSH_DE : return "PUSH DE";
    case OP_PUSH_HL : return "PUSH HL";
    case OP_PUSH_AF : return "PUSH AF";
    case OP_POP_BC  : return "POP BC";
    case OP_POP_DE  : return "POP DE";
    case OP_POP_HL  : return "POP HL";
    case OP_POP_AF  : return "POP AF";

    case OP_LOAD_HL_SP_E8 : return "LD HL, SP+e8";

    // - - - 8 bit Add 
    case OP_ADD_A_B : return "ADD A, B";
    case OP_ADD_A_C : return "ADD A, C";
    case OP_ADD_A_D : return "ADD A, D";
    case OP_ADD_A_E : return "ADD A, E";
    case OP_ADD_A_H : return "ADD A, H";
    case OP_ADD_A_L : return "ADD A, L";
    case OP_ADD_A_A : return "ADD A, A";

    // - - - indirect A adds 
    case OP_ADD_A_HL        : return "ADD [HL]";
    case OP_ADD_A_8_BIT_IMM : return "ADD n";

    // - - - adc reg 
    case OP_ADC_A_B : return "ADC B";
    case OP_ADC_A_C : return "ADC C";
    case OP_ADC_A_D : return "ADC D";
    case OP_ADC_A_E : return "ADC E";
    case OP_ADC_A_H : return "ADC H";
    case OP_ADC_A_L : return "ADC L";
    case OP_ADC_A_A : return "ADC A";

    // - - - indirect add c 
    case OP_ADC_A_HL        : return "ADC [HL]";
    case OP_ADC_A_8_BIT_IMM : return "ADC n";

    // - - - SUB reg 
    case OP_SUB_A_B : return "SUB B";
    case OP_SUB_A_C : return "SUB C";
    case OP_SUB_A_D : return "SUB D";
    case OP_SUB_A_E : return "SUB E";
    case OP_SUB_A_H : return "SUB H";
    case OP_SUB_A_L : return "SUB L";
    case OP_SUB_A_A : return "SUB A";

    // - - - SUB indirects 
    case OP_SUB_A_HL        : return "SUB [HL]";
    case OP_SUB_A_8_BIT_IMM : return "SUB n";

    // - - - SBC reg 
    case OP_SBC_A_B : return "SBC B";
    case OP_SBC_A_C : return "SBC C";
    case OP_SBC_A_D : return "SBC D";
    case OP_SBC_A_E : return "SBC E";
    case OP_SBC_A_H : return "SBC H";
    case OP_SBC_A_L : return "SBC L";
    case OP_SBC_A_A : return "SBC A";

    case OP_SBC_A_HL        : return "SBC [HL]";
    case OP_SBC_A_8_BIT_IMM : return "SBC n";

    // - - - 8 bit compare Reg 
    case OP_COMP_A_B : return "CP B";
    case OP_COMP_A_C : return "CP C";
    case OP_COMP_A_D : return "CP D";
    case OP_COMP_A_E : return "CP E";
    case OP_COMP_A_H : return "CP H";
    case OP_COMP_A_L : return "CP L";
    case OP_COMP_A_A : return "CP A";

    case OP_COMP_A_HL        : return "CP [HL]";
    case OP_COMP_A_8_BIT_IMM : return "CP n";

    // - - - 8 bit increments 
    case OP_INC_B : return "INC B";
    case OP_INC_C : return "INC C";
    case OP_INC_D : return "INC D";
    case OP_INC_E : return "INC E";
    case OP_INC_H : return "INC H";
    case OP_INC_L : return "INC L";
    case OP_INC_A : return "INC A";

    // - - - 8 bit decrements 
    case OP_DEC_B : return "DEC B";
    case OP_DEC_C : return "DEC C";
    case OP_DEC_D : return "DEC D";
    case OP_DEC_E : return "DEC E";
    case OP_DEC_H : return "DEC H";
    case OP_DEC_L : return "DEC L";
    case OP_DEC_A : return "DEC A";

    // - - - indirect incr / decr 
    case OP_DEC_HL : return "DEC [HL]";
    case OP_INC_HL : return "INC [HL]";

    // - - - 8 bit AND 
    case OP_AND_A_B : return "AND B";
    case OP_AND_A_C : return "AND C";
    case OP_AND_A_D : return "AND D";
    case OP_AND_A_E : return "AND E";
    case OP_AND_A_H : return "AND H";
    case OP_AND_A_L : return "AND L";
    case OP_AND_A_A : return "AND A";

    case OP_AND_A_HL        : return "AND [HL]";
    case OP_AND_A_8_BIT_IMM : return "AND n";

    // - - - 8 bit OR 
    case OP_OR_A_B : return "OR B";
    case OP_OR_A_C : return "OR C";
    case OP_OR_A_D : return "OR D";
    case OP_OR_A_E : return "OR E";
    case OP_OR_A_H : return "OR H";
    case OP_OR_A_L : return "OR L";
    case OP_OR_A_A : return "OR A";

    // - - - Indirect OR 
    case OP_OR_A_HL        : return "OP [HL]";
    case OP_OR_A_8_BIT_IMM : return "OP n";

    // - - - 8 bit XOR 
    case OP_XOR_A_B : return "XOR B";
    case OP_XOR_A_C : return "XOR C";
    case OP_XOR_A_D : return "XOR D";
    case OP_XOR_A_E : return "XOR E";
    case OP_XOR_A_H : return "XOR H";
    case OP_XOR_A_L : return "XOR L";
    case OP_XOR_A_A : return "XOR A";

    case OP_XOR_A_HL        : return "XOR [HL]";
    case OP_XOR_A_8_BIT_IMM : return "XOR n";

    // - - - extra arithmetic 
    case OP_CCF : return "CCF";
    case OP_SCF : return "SCF";
    case OP_DAA : return "DAA";
    case OP_CPL : return "CPL";

    // - - - 16 bit increment 
    case OP_INC_BC      : return "INC BC";
    case OP_INC_DE      : return "INC DE";
    case OP_INC_HL_REG  : return "INC HL";
    case OP_INC_SP      : return "INC SP";

    case OP_DEC_BC      : return "DEC BC";
    case OP_DEC_DE      : return "DEC DE";
    case OP_DEC_HL_REG  : return "DEC HL";
    case OP_DEC_SP      : return "DEC SP";

    // - - - 16 bit add 
    case OP_ADD_HL_BC : return "ADD HL, BC";
    case OP_ADD_HL_DE : return "ADD HL, DE";
    case OP_ADD_HL_HL : return "ADD HL, HL";
    case OP_ADD_HL_SP : return "ADD HL, SP";
    case OP_ADD_SP_E8 : return "ADD SP+e8";

    // - - - roate 
    case OP_ROTATE_LEFT_CIRCULAR_A    : return "RLCA";
    case OP_ROTATE_RIGHT_CIRCULAR_A   : return "RRCA";
    case OP_ROTATE_LEFT_A             : return "RLA";
    case OP_ROTATE_RIGHT_A            : return "RRA";

    // - - - CB rotates left circular
    case OP_CB_ROTATE_LEFT_CIRCULAR_B : return "RLC B";
    case OP_CB_ROTATE_LEFT_CIRCULAR_C : return "RLC C";
    case OP_CB_ROTATE_LEFT_CIRCULAR_D : return "RLC D";
    case OP_CB_ROTATE_LEFT_CIRCULAR_E : return "RLC E";
    case OP_CB_ROTATE_LEFT_CIRCULAR_H : return "RLC H";
    case OP_CB_ROTATE_LEFT_CIRCULAR_L : return "RLC L";
    case OP_CB_ROTATE_LEFT_CIRCULAR_A : return "RLC A";
    case OP_CB_ROTATE_LEFT_CIRCULAR_HL: return "RLC [HL]";

    // - - - CB rotates right circular
    case OP_CB_ROTATE_RIGHT_CIRCULAR_B : return "RRC B";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_C : return "RRC C";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_D : return "RRC D";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_E : return "RRC E";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_H : return "RRC H";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_L : return "RRC L";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_A : return "RRC A";
    case OP_CB_ROTATE_RIGHT_CIRCULAR_HL: return "RRC [HL]";

    // - - - CB rotates left 
    case OP_CB_ROTATE_LEFT_B : return "RL B";
    case OP_CB_ROTATE_LEFT_C : return "RL C";
    case OP_CB_ROTATE_LEFT_D : return "RL D";
    case OP_CB_ROTATE_LEFT_E : return "RL E";
    case OP_CB_ROTATE_LEFT_H : return "RL H";
    case OP_CB_ROTATE_LEFT_L : return "RL L";
    case OP_CB_ROTATE_LEFT_A : return "RL A";
    case OP_CB_ROTATE_LEFT_HL: return "RL [HL]";

    // - - - CB rotate right 
    case OP_CB_ROTATE_RIGHT_B : return "RR B";
    case OP_CB_ROTATE_RIGHT_C : return "RR C";
    case OP_CB_ROTATE_RIGHT_D : return "RR D";
    case OP_CB_ROTATE_RIGHT_E : return "RR E";
    case OP_CB_ROTATE_RIGHT_H : return "RR H";
    case OP_CB_ROTATE_RIGHT_L : return "RR L";
    case OP_CB_ROTATE_RIGHT_A : return "RR A";
    case OP_CB_ROTATE_RIGHT_HL: return "RR [HL]";

    // - - - Cb shift left arith 
    case OP_CB_SHIFT_LEFT_ARITH_B : return "SLA B";
    case OP_CB_SHIFT_LEFT_ARITH_C : return "SLA C";
    case OP_CB_SHIFT_LEFT_ARITH_D : return "SLA D";
    case OP_CB_SHIFT_LEFT_ARITH_E : return "SLA E";
    case OP_CB_SHIFT_LEFT_ARITH_H : return "SLA H";
    case OP_CB_SHIFT_LEFT_ARITH_L : return "SLA L";
    case OP_CB_SHIFT_LEFT_ARITH_A : return "SLA A";
    case OP_CB_SHIFT_LEFT_ARITH_HL: return "SLA [HL]";

    // - - - cb Shift right arith 
    case OP_CB_SHIFT_RIGHT_ARITH_B : return "SRA B"; 
    case OP_CB_SHIFT_RIGHT_ARITH_C : return "SRA C";
    case OP_CB_SHIFT_RIGHT_ARITH_D : return "SRA D";
    case OP_CB_SHIFT_RIGHT_ARITH_E : return "SRA E";
    case OP_CB_SHIFT_RIGHT_ARITH_H : return "SRA H";
    case OP_CB_SHIFT_RIGHT_ARITH_L : return "SRA L";
    case OP_CB_SHIFT_RIGHT_ARITH_A : return "SRA A";
    case OP_CB_SHIFT_RIGHT_ARITH_HL: return "SRA [HL]";

    // - --  swap 
    case OP_CB_SWAP_B : return "SWAP B";
    case OP_CB_SWAP_C : return "SWAP C";
    case OP_CB_SWAP_D : return "SWAP D";
    case OP_CB_SWAP_E : return "SWAP E";
    case OP_CB_SWAP_H : return "SWAP H";
    case OP_CB_SWAP_L : return "SWAP L";
    case OP_CB_SWAP_A : return "SWAP A";
    case OP_CB_SWAP_HL: return "SWAP [HL]";

    // - - - cb shift right logic 
    case OP_CB_SHIFT_RIGHT_LOGIC_B : return "SRL B";
    case OP_CB_SHIFT_RIGHT_LOGIC_C : return "SRL C";
    case OP_CB_SHIFT_RIGHT_LOGIC_D : return "SRL D";
    case OP_CB_SHIFT_RIGHT_LOGIC_E : return "SRL E";
    case OP_CB_SHIFT_RIGHT_LOGIC_H : return "SRL H";
    case OP_CB_SHIFT_RIGHT_LOGIC_L : return "SRL L";
    case OP_CB_SHIFT_RIGHT_LOGIC_A : return "SRL A";
    case OP_CB_SHIFT_RIGHT_LOGIC_HL: return "SRL [HL]";

    // - - - Bit read 
    case OP_CB_BIT_0_B : return "BIT 0, B";
    case OP_CB_BIT_0_C : return "BIT 0, C";
    case OP_CB_BIT_0_D : return "BIT 0, D";
    case OP_CB_BIT_0_E : return "BIT 0, E";
    case OP_CB_BIT_0_H : return "BIT 0, H";
    case OP_CB_BIT_0_L : return "BIT 0, L";
    case OP_CB_BIT_0_A : return "BIT 0, A";
    case OP_CB_BIT_0_HL: return "BIT 0, [HL]";

    case OP_CB_BIT_1_B : return "BIT 1, B";
    case OP_CB_BIT_1_C : return "BIT 1, C";
    case OP_CB_BIT_1_D : return "BIT 1, D";
    case OP_CB_BIT_1_E : return "BIT 1, E";
    case OP_CB_BIT_1_H : return "BIT 1, H";
    case OP_CB_BIT_1_L : return "BIT 1, L";
    case OP_CB_BIT_1_A : return "BIT 1, A";
    case OP_CB_BIT_1_HL: return "BIT 1, [HL]";

    case OP_CB_BIT_2_B : return "BIT 2, B";
    case OP_CB_BIT_2_C : return "BIT 2, C";
    case OP_CB_BIT_2_D : return "BIT 2, D";
    case OP_CB_BIT_2_E : return "BIT 2, E";
    case OP_CB_BIT_2_H : return "BIT 2, H";
    case OP_CB_BIT_2_L : return "BIT 2, L";
    case OP_CB_BIT_2_A : return "BIT 2, A";
    case OP_CB_BIT_2_HL: return "BIT 2, [HL]";

    case OP_CB_BIT_3_B : return "BIT 3, B";
    case OP_CB_BIT_3_C : return "BIT 3, C";
    case OP_CB_BIT_3_D : return "BIT 3, D";
    case OP_CB_BIT_3_E : return "BIT 3, E";
    case OP_CB_BIT_3_H : return "BIT 3, H";
    case OP_CB_BIT_3_L : return "BIT 3, L";
    case OP_CB_BIT_3_A : return "BIT 3, A";
    case OP_CB_BIT_3_HL: return "BIT 3, [HL]";

    case OP_CB_BIT_4_B : return "BIT 4, B";
    case OP_CB_BIT_4_C : return "BIT 4, C";
    case OP_CB_BIT_4_D : return "BIT 4, D";
    case OP_CB_BIT_4_E : return "BIT 4, E";
    case OP_CB_BIT_4_H : return "BIT 4, H";
    case OP_CB_BIT_4_L : return "BIT 4, L";
    case OP_CB_BIT_4_A : return "BIT 4, A";
    case OP_CB_BIT_4_HL: return "BIT 4, [HL]";

    case OP_CB_BIT_5_B : return "BIT 5, B";
    case OP_CB_BIT_5_C : return "BIT 5, C";
    case OP_CB_BIT_5_D : return "BIT 5, D";
    case OP_CB_BIT_5_E : return "BIT 5, E";
    case OP_CB_BIT_5_H : return "BIT 5, H";
    case OP_CB_BIT_5_L : return "BIT 5, L";
    case OP_CB_BIT_5_A : return "BIT 5, A";
    case OP_CB_BIT_5_HL: return "BIT 5, [HL]";

    case OP_CB_BIT_6_B : return "BIT 6, B";
    case OP_CB_BIT_6_C : return "BIT 6, C";
    case OP_CB_BIT_6_D : return "BIT 6, D";
    case OP_CB_BIT_6_E : return "BIT 6, E";
    case OP_CB_BIT_6_H : return "BIT 6, H";
    case OP_CB_BIT_6_L : return "BIT 6, L";
    case OP_CB_BIT_6_A : return "BIT 6, A";
    case OP_CB_BIT_6_HL: return "BIT 6, [HL]";

    case OP_CB_BIT_7_B : return "BIT 7, B";
    case OP_CB_BIT_7_C : return "BIT 7, C";
    case OP_CB_BIT_7_D : return "BIT 7, D";
    case OP_CB_BIT_7_E : return "BIT 7, E";
    case OP_CB_BIT_7_H : return "BIT 7, H";
    case OP_CB_BIT_7_L : return "BIT 7, L";
    case OP_CB_BIT_7_A : return "BIT 7, A";
    case OP_CB_BIT_7_HL: return "BIT 7, [HL]";

    // - - - Bit reset
    case OP_CB_RESET_0_B : return "RES 0, B";
    case OP_CB_RESET_0_C : return "RES 0, C";
    case OP_CB_RESET_0_D : return "RES 0, D";
    case OP_CB_RESET_0_E : return "RES 0, E";
    case OP_CB_RESET_0_H : return "RES 0, H";
    case OP_CB_RESET_0_L : return "RES 0, L";
    case OP_CB_RESET_0_A : return "RES 0, A";
    case OP_CB_RESET_0_HL: return "RES 0, [HL]";

    case OP_CB_RESET_1_B : return "RES 1, B";
    case OP_CB_RESET_1_C : return "RES 1, C";
    case OP_CB_RESET_1_D : return "RES 1, D";
    case OP_CB_RESET_1_E : return "RES 1, E";
    case OP_CB_RESET_1_H : return "RES 1, H";
    case OP_CB_RESET_1_L : return "RES 1, L";
    case OP_CB_RESET_1_A : return "RES 1, A";
    case OP_CB_RESET_1_HL: return "RES 1, [HL]";

    case OP_CB_RESET_2_B : return "RES 2, B";
    case OP_CB_RESET_2_C : return "RES 2, C";
    case OP_CB_RESET_2_D : return "RES 2, D";
    case OP_CB_RESET_2_E : return "RES 2, E";
    case OP_CB_RESET_2_H : return "RES 2, H";
    case OP_CB_RESET_2_L : return "RES 2, L";
    case OP_CB_RESET_2_A : return "RES 2, A";
    case OP_CB_RESET_2_HL: return "RES 2, [HL]";

    case OP_CB_RESET_3_B : return "RES 3, B";
    case OP_CB_RESET_3_C : return "RES 3, C";
    case OP_CB_RESET_3_D : return "RES 3, D";
    case OP_CB_RESET_3_E : return "RES 3, E";
    case OP_CB_RESET_3_H : return "RES 3, H";
    case OP_CB_RESET_3_L : return "RES 3, L";
    case OP_CB_RESET_3_A : return "RES 3, A";
    case OP_CB_RESET_3_HL: return "RES 3, [HL]";

    case OP_CB_RESET_4_B : return "RES 4, B";
    case OP_CB_RESET_4_C : return "RES 4, C";
    case OP_CB_RESET_4_D : return "RES 4, D";
    case OP_CB_RESET_4_E : return "RES 4, E";
    case OP_CB_RESET_4_H : return "RES 4, H";
    case OP_CB_RESET_4_L : return "RES 4, L";
    case OP_CB_RESET_4_A : return "RES 4, A";
    case OP_CB_RESET_4_HL: return "RES 4, [HL]";

    case OP_CB_RESET_5_B : return "RES 5, B";
    case OP_CB_RESET_5_C : return "RES 5, C";
    case OP_CB_RESET_5_D : return "RES 5, D";
    case OP_CB_RESET_5_E : return "RES 5, E";
    case OP_CB_RESET_5_H : return "RES 5, H";
    case OP_CB_RESET_5_L : return "RES 5, L";
    case OP_CB_RESET_5_A : return "RES 5, A";
    case OP_CB_RESET_5_HL: return "RES 5, [HL]";

    case OP_CB_RESET_6_B : return "RES 6, B";
    case OP_CB_RESET_6_C : return "RES 6, C";
    case OP_CB_RESET_6_D : return "RES 6, D";
    case OP_CB_RESET_6_E : return "RES 6, E";
    case OP_CB_RESET_6_H : return "RES 6, H";
    case OP_CB_RESET_6_L : return "RES 6, L";
    case OP_CB_RESET_6_A : return "RES 6, A";
    case OP_CB_RESET_6_HL: return "RES 6, [HL]";

    case OP_CB_RESET_7_B : return "RES 7, B";
    case OP_CB_RESET_7_C : return "RES 7, C";
    case OP_CB_RESET_7_D : return "RES 7, D";
    case OP_CB_RESET_7_E : return "RES 7, E";
    case OP_CB_RESET_7_H : return "RES 7, H";
    case OP_CB_RESET_7_L : return "RES 7, L";
    case OP_CB_RESET_7_A : return "RES 7, A";
    
    case OP_CB_RESET_7_HL: return "RES 7, [HL]";
    
    // - - - Bit set
    case OP_CB_SET_0_B : return "SET 0, B";
    case OP_CB_SET_0_C : return "SET 0, C";
    case OP_CB_SET_0_D : return "SET 0, D";
    case OP_CB_SET_0_E : return "SET 0, E";
    case OP_CB_SET_0_H : return "SET 0, H";
    case OP_CB_SET_0_L : return "SET 0, L";
    case OP_CB_SET_0_A : return "SET 0, A";
    case OP_CB_SET_0_HL: return "SET 0, [HL]";

    case OP_CB_SET_1_B : return "SET 1, B";
    case OP_CB_SET_1_C : return "SET 1, C";
    case OP_CB_SET_1_D : return "SET 1, D";
    case OP_CB_SET_1_E : return "SET 1, E";
    case OP_CB_SET_1_H : return "SET 1, H";
    case OP_CB_SET_1_L : return "SET 1, L";
    case OP_CB_SET_1_A : return "SET 1, A";
    case OP_CB_SET_1_HL: return "SET 1, [HL]";

    case OP_CB_SET_2_B : return "SET 2, B";
    case OP_CB_SET_2_C : return "SET 2, C";
    case OP_CB_SET_2_D : return "SET 2, D";
    case OP_CB_SET_2_E : return "SET 2, E";
    case OP_CB_SET_2_H : return "SET 2, H";
    case OP_CB_SET_2_L : return "SET 2, L";
    case OP_CB_SET_2_A : return "SET 2, A";
    case OP_CB_SET_2_HL: return "SET 2, [HL]";

    case OP_CB_SET_3_B : return "SET 3, B";
    case OP_CB_SET_3_C : return "SET 3, C";
    case OP_CB_SET_3_D : return "SET 3, D";
    case OP_CB_SET_3_E : return "SET 3, E";
    case OP_CB_SET_3_H : return "SET 3, H";
    case OP_CB_SET_3_L : return "SET 3, L";
    case OP_CB_SET_3_A : return "SET 3, A";
    case OP_CB_SET_3_HL: return "SET 3, [HL]";

    case OP_CB_SET_4_B : return "SET 4, B";
    case OP_CB_SET_4_C : return "SET 4, C";
    case OP_CB_SET_4_D : return "SET 4, D";
    case OP_CB_SET_4_E : return "SET 4, E";
    case OP_CB_SET_4_H : return "SET 4, H";
    case OP_CB_SET_4_L : return "SET 4, L";
    case OP_CB_SET_4_A : return "SET 4, A";
    case OP_CB_SET_4_HL: return "SET 4, [HL]";

    case OP_CB_SET_5_B : return "SET 5, B";
    case OP_CB_SET_5_C : return "SET 5, C";
    case OP_CB_SET_5_D : return "SET 5, D";
    case OP_CB_SET_5_E : return "SET 5, E";
    case OP_CB_SET_5_H : return "SET 5, H";
    case OP_CB_SET_5_L : return "SET 5, L";
    case OP_CB_SET_5_A : return "SET 5, A";
    case OP_CB_SET_5_HL: return "SET 5, [HL]";

    case OP_CB_SET_6_B : return "SET 6, B";
    case OP_CB_SET_6_C : return "SET 6, C";
    case OP_CB_SET_6_D : return "SET 6, D";
    case OP_CB_SET_6_E : return "SET 6, E";
    case OP_CB_SET_6_H : return "SET 6, H";
    case OP_CB_SET_6_L : return "SET 6, L";
    case OP_CB_SET_6_A : return "SET 6, A";
    case OP_CB_SET_6_HL: return "SET 6, [HL]";

    case OP_CB_SET_7_B : return "SET 7, B";
    case OP_CB_SET_7_C : return "SET 7, C";
    case OP_CB_SET_7_D : return "SET 7, D";
    case OP_CB_SET_7_E : return "SET 7, E";
    case OP_CB_SET_7_H : return "SET 7, H";
    case OP_CB_SET_7_L : return "SET 7, L";
    case OP_CB_SET_7_A : return "SET 7, A";
    case OP_CB_SET_7_HL: return "SET 7, [HL]";

    // - - - Jump instructions 
    case OP_JUMP_16_BIT_IMM           : return "JP nn";
    case OP_JUMP_HL                   : return "JP HL";
    case OP_JUMP_NZ_16_BIT_IMM        : return "JP NZ, nn";
    case OP_JUMP_Z_16_BIT_IMM         : return "JP Z, nn";
    case OP_JUMP_NC_16_BIT_IMM        : return "JP NC, nn";
    case OP_JUMP_C_16_BIT_IMM         : return "JP C, nn";
    case OP_JUMP_SIGNED_8_BIT_IMM     : return "JR e";
    case OP_JUMP_NZ_SIGNED_8_BIT_IMM  : return "JR NZ, e";
    case OP_JUMP_Z_SIGNED_8_BIT_IMM   : return "JR Z, e";
    case OP_JUMP_NC_SIGNED_8_BIT_IMM  : return "JR NC, e";
    case OP_JUMP_C_SIGNED_8_BIT_IMM   : return "JR C, e";

    // - - - Call instruction 
    case OP_CALL_16_BIT_IMM     : return "CALL nn";
    case OP_CALL_NZ_16_BIT_IMM  : return "CALL NZ, nn";
    case OP_CALL_Z_16_BIT_IMM   : return "CALL Z, nn";
    case OP_CALL_NC_16_BIT_IMM  : return "CALL NC, nn";
    case OP_CALL_C_16_BIT_IMM   : return "CALL C, nn";

    // - - - Return instruction 
    case OP_RETURN            : return "RET";
    case OP_RETURN_NZ         : return "RET NZ";
    case OP_RETURN_Z          : return "RET Z";
    case OP_RETURN_NC         : return "RET NC";
    case OP_RETURN_C          : return "RET C";
    case OP_RETURN_INTERRUPT  : return "RETI";

    // - - - restart instruction 
    case OP_RESTART_00 : return "RST 00";
    case OP_RESTART_08 : return "RST 08";
    case OP_RESTART_10 : return "RST 10";
    case OP_RESTART_18 : return "RST 18";
    case OP_RESTART_20 : return "RST 20";
    case OP_RESTART_28 : return "RST 28";
    case OP_RESTART_30 : return "RST 30";
    case OP_RESTART_38 : return "RST 38";

    // - - - misc 
    case OP_DISABLE_INTERRUPT : return "DI";
    case OP_ENABLE_INTERRUPT  : return "EI";
    case OP_STOP              : return "STOP";
    case OP_HALT              : return "HALT";
  }
}

const char* instructionGetRegName(RegType REG)
{
  switch (REG)
  {
    case RT_NONE: return "";

    case RT_A: return "A";
    case RT_B: return "B";
    case RT_C: return "C";
    case RT_D: return "D";
    case RT_E: return "E";
    case RT_H: return "H";
    case RT_L: return "L";

    case RT_AF: return "AF";
    case RT_BC: return "BC";
    case RT_DE: return "DE";
    case RT_HL: return "HL";
    case RT_SP: return "SP";
    case RT_PC: return "PC";

    default:    return "?";
  }
}
