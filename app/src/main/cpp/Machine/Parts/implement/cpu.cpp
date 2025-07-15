#include "../include/cpu.h"
#include "../include/bus.h"
#include "../../../GameBoyCore.h"
#include "../../../ForgeLibrary/include/asserts.h"
#include "../../../ForgeLibrary/include/logger.h"

static CPUContext   cpuCTX    = {0};
static int          emuCycles = 0;
#define POSSIBLE_INSTR_COUNT  0x100


// - - - Instructions - - - 

static Instruction instructions[POSSIBLE_INSTR_COUNT] =
  {
    // - - - Hex because easy to reference here: https://meganesulli.com/static/851d34afbc4673ee915a8233fda67922/78d47/opcode-tables-screenshot.png

    // - - - 0x0X
    [0x00] = {INSTR_NOP,    ADDR_MODE_IMP},
    [0x01] = {INSTR_LOAD,   ADDR_MODE_R_D16, REG_BC},
    [0x02] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_BC,   REG_A},
    [0x03] = {INSTR_INC,    ADDR_MODE_R,     REG_BC},
    [0x04] = {INSTR_INC,    ADDR_MODE_R,     REG_B},
    [0x05] = {INSTR_DEC,    ADDR_MODE_R,     REG_B},
    [0x06] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_B},
    [0x08] = {INSTR_LOAD,   ADDR_MODE_A16_R, REG_NONE, REG_SP},
    [0x09] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_HL,   REG_BC},
    [0x0A] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_A,    REG_BC},
    [0x0B] = {INSTR_DEC,    ADDR_MODE_R,     REG_BC},
    [0x0C] = {INSTR_INC,    ADDR_MODE_R,     REG_C},
    [0x0D] = {INSTR_DEC,    ADDR_MODE_R,     REG_C},
    [0x0E] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_C},

    // - - - 0x1X
    [0x11] = {INSTR_LOAD,   ADDR_MODE_R_D16, REG_DE},
    [0x12] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_DE,   REG_A},
    [0x13] = {INSTR_INC,    ADDR_MODE_R,     REG_DE},
    [0x14] = {INSTR_INC,    ADDR_MODE_R,     REG_D},
    [0x15] = {INSTR_DEC,    ADDR_MODE_R,     REG_D},
    [0x16] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_D},
    [0x18] = {INSTR_JR,     ADDR_MODE_D8},
    [0x19] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_HL,   REG_DE},
    [0x1A] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_A,    REG_DE},
    [0x1B] = {INSTR_DEC,    ADDR_MODE_R,     REG_DE},
    [0x1C] = {INSTR_INC,    ADDR_MODE_R,     REG_E},
    [0x1D] = {INSTR_DEC,    ADDR_MODE_R,     REG_E},
    [0x1E] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_E},

    // - - - 0x2X
    [0x20] = {INSTR_JR,     ADDR_MODE_D8,    REG_NONE, REG_NONE, CHECK_NOT_ZERO},
    [0x21] = {INSTR_LOAD,   ADDR_MODE_R_D16, REG_HL},
    [0x22] = {INSTR_LOAD,   ADDR_MODE_HLI_R, REG_HL,   REG_A},
    [0x23] = {INSTR_INC,    ADDR_MODE_R,     REG_HL},
    [0x24] = {INSTR_INC,    ADDR_MODE_R,     REG_H},
    [0x25] = {INSTR_DEC,    ADDR_MODE_R,     REG_H},
    [0x26] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_H},
    [0x28] = {INSTR_JR,     ADDR_MODE_D8,    REG_NONE, REG_NONE, CHECK_ZERO},
    [0x29] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_HL,   REG_HL},
    [0x2A] = {INSTR_LOAD,   ADDR_MODE_R_HLI, REG_A,    REG_HL},
    [0x2B] = {INSTR_DEC,    ADDR_MODE_R,     REG_HL},
    [0x2C] = {INSTR_INC,    ADDR_MODE_R,     REG_L},
    [0x2D] = {INSTR_DEC,    ADDR_MODE_R,     REG_L},
    [0x2E] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_L},

    // - - - 0x3X
    [0x30] = {INSTR_JR,     ADDR_MODE_D8,    REG_NONE, REG_NONE, CHECK_NO_CARRY},
    [0x31] = {INSTR_LOAD,   ADDR_MODE_R_D16, REG_SP},
    [0x32] = {INSTR_LOAD,   ADDR_MODE_HLD_R, REG_HL,   REG_A},
    [0x33] = {INSTR_INC,    ADDR_MODE_R,     REG_SP},
    [0x34] = {INSTR_INC,    ADDR_MODE_R,     REG_HL},
    [0x35] = {INSTR_DEC,    ADDR_MODE_R,     REG_HL},
    [0x36] = {INSTR_LOAD,   ADDR_MODE_MR_D8, REG_HL},
    [0x38] = {INSTR_JR,     ADDR_MODE_D8,    REG_NONE, REG_NONE, CHECK_NOT_ZERO},
    [0x39] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_HL,   REG_SP},
    [0x3A] = {INSTR_LOAD,   ADDR_MODE_R_HLD, REG_A,    REG_HL},
    [0x3B] = {INSTR_DEC,    ADDR_MODE_R,     REG_SP},
    [0x3C] = {INSTR_INC,    ADDR_MODE_R,     REG_A},
    [0x3D] = {INSTR_DEC,    ADDR_MODE_R,     REG_A},
    [0x3E] = {INSTR_LOAD,   ADDR_MODE_R_D8,  REG_A},

    // - - - 0x4X
    [0x40] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_B},
    [0x41] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_C},
    [0x42] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_D},
    [0x43] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_E},
    [0x44] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_H},
    [0x45] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_L},
    [0x46] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_B,    REG_HL},
    [0x47] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_B,    REG_A},
    [0x48] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_B},
    [0x49] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_C},
    [0x4A] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_D},
    [0x4B] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_E},
    [0x4C] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_H},
    [0x4D] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_L},
    [0x4E] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_C,    REG_HL},
    [0x4F] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_C,    REG_A},

    // - - - 0x5X
    [0x50] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_B},
    [0x51] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_C},
    [0x52] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_D},
    [0x53] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_E},
    [0x54] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_H},
    [0x55] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_L},
    [0x56] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_D,    REG_HL},
    [0x57] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_D,    REG_A},
    [0x58] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_B},
    [0x59] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_C},
    [0x5A] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_D},
    [0x5B] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_E},
    [0x5C] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_H},
    [0x5D] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_L},
    [0x5E] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_E,    REG_HL},
    [0x5F] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_E,    REG_A},

    // - - - 0x6X
    [0x60] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_B},
    [0x61] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_C},
    [0x62] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_D},
    [0x63] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_E},
    [0x64] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_H},
    [0x65] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_L},
    [0x66] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_H,    REG_HL},
    [0x67] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_H,    REG_A},
    [0x68] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_B},
    [0x69] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_C},
    [0x6A] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_D},
    [0x6B] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_E},
    [0x6C] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_H},
    [0x6D] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_L},
    [0x6E] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_L,    REG_HL},
    [0x6F] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_L,    REG_A},

    // - - - 0x7X
    [0x70] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_B},
    [0x71] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_C},
    [0x72] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_D},
    [0x73] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_E},
    [0x74] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_H},
    [0x75] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_L},
    [0x76] = {INSTR_HALT},
    [0x77] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_HL,   REG_A},
    [0x78] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_B},
    [0x79] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_C},
    [0x7A] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_D},
    [0x7B] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_E},
    [0x7C] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_H},
    [0x7D] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_L},
    [0x7E] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_A,    REG_HL},
    [0x7F] = {INSTR_LOAD,   ADDR_MODE_R_R,   REG_A,    REG_A},
    [0xAF] = {INSTR_XOR,    ADDR_MODE_R,     REG_A},

    // - - - 0x8X 
    [0x80] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_B},
    [0x81] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_C},
    [0x82] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_D},
    [0x83] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_E},
    [0x84] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_H},
    [0x85] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_L},
    [0x86] = {INSTR_ADD,    ADDR_MODE_R_MR,  REG_A,    REG_HL},
    [0x87] = {INSTR_ADD,    ADDR_MODE_R_R,   REG_A,    REG_A},
    [0x88] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_B},
    [0x89] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_C},
    [0x8A] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_D},
    [0x8B] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_E},
    [0x8C] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_H},
    [0x8D] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_L},
    [0x8E] = {INSTR_ADC,    ADDR_MODE_R_MR,  REG_A,    REG_HL},
    [0x8F] = {INSTR_ADC,    ADDR_MODE_R_R,   REG_A,    REG_A},

    // - - - 0x9X
    [0x90] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_B},
    [0x91] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_C},
    [0x92] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_D},
    [0x93] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_E},
    [0x94] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_H},
    [0x95] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_L},
    [0x96] = {INSTR_SUB,    ADDR_MODE_R_MR,  REG_A,    REG_HL},
    [0x97] = {INSTR_SUB,    ADDR_MODE_R_R,   REG_A,    REG_A},
    [0x98] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_B},
    [0x99] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_C},
    [0x9A] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_D},
    [0x9B] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_E},
    [0x9C] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_H},
    [0x9D] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_L},
    [0x9E] = {INSTR_SBC,    ADDR_MODE_R_MR,  REG_A,    REG_HL},
    [0x9F] = {INSTR_SBC,    ADDR_MODE_R_R,   REG_A,    REG_A},

    // - - - 0xCX
    [0xC0] = {INSTR_RET,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NOT_ZERO},
    [0xC1] = {INSTR_POP,    ADDR_MODE_R,     REG_BC},
    [0xC2] = {INSTR_JUMP,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_NOT_ZERO},
    [0xC3] = {INSTR_JUMP,   ADDR_MODE_D16},
    [0xC4] = {INSTR_CALL,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_NOT_ZERO},
    [0xC5] = {INSTR_PUSH,   ADDR_MODE_R,     REG_BC},
    [0xC6] = {INSTR_ADD,    ADDR_MODE_R_A8,  REG_A},
    [0xC7] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x00},
    [0xC8] = {INSTR_RET,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_ZERO},
    [0xC9] = {INSTR_RET},
    [0xCA] = {INSTR_JUMP,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_ZERO},
    [0xCE] = {INSTR_ADC,    ADDR_MODE_R_D8,  REG_A},
    [0xCC] = {INSTR_CALL,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_ZERO},
    [0xCD] = {INSTR_CALL,   ADDR_MODE_D16},
    [0xCF] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x08},

    // - - - 0xDX 
    [0xD0] = {INSTR_RET,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NO_CARRY},
    [0xD1] = {INSTR_POP,    ADDR_MODE_R,     REG_DE},
    [0xD2] = {INSTR_JUMP,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_NO_CARRY},
    [0xD4] = {INSTR_CALL,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_NO_CARRY},
    [0xD5] = {INSTR_PUSH,   ADDR_MODE_R,     REG_DE},
    [0xD7] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x10},
    [0xD8] = {INSTR_RET,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_CARRY},
    [0xD9] = {INSTR_RETI},
    [0xDA] = {INSTR_JUMP,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_CARRY},
    [0xDC] = {INSTR_CALL,   ADDR_MODE_D16,   REG_NONE, REG_NONE, CHECK_CARRY},
    [0xDF] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x18},

    // - - - 0xEX
    [0xE0] = {INSTR_LDH,    ADDR_MODE_A8_R,  REG_NONE, REG_A},
    [0xE1] = {INSTR_POP,    ADDR_MODE_R,     REG_HL},
    [0xE2] = {INSTR_LOAD,   ADDR_MODE_MR_R,  REG_C,    REG_A},
    [0xE5] = {INSTR_PUSH,   ADDR_MODE_R,     REG_HL},
    [0xE7] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x20},
    [0xE8] = {INSTR_ADD,    ADDR_MODE_R_D8,  REG_SP},
    [0xE9] = {INSTR_JUMP,   ADDR_MODE_MR,    REG_HL},
    [0xEA] = {INSTR_LOAD,   ADDR_MODE_A16_R, REG_NONE, REG_A},
    [0xEF] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x28},

    // - - - 0xFX
    [0xF0] = {INSTR_LDH,    ADDR_MODE_R_A8,  REG_A},
    [0xF1] = {INSTR_POP,    ADDR_MODE_R,     REG_AF},
    [0xF2] = {INSTR_LOAD,   ADDR_MODE_R_MR,  REG_A,    REG_C},
    [0xF3] = {INSTR_DI},
    [0xF5] = {INSTR_PUSH,   ADDR_MODE_R,     REG_AF},
    [0xF7] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x30},
    [0xFA] = {INSTR_LOAD,   ADDR_MODE_R_A16, REG_A},
    [0xFF] = {INSTR_RST,    ADDR_MODE_IMP,   REG_NONE, REG_NONE, CHECK_NONE,     0x38},
  };

std::string inst_lookup[] = 
  {
    "<NONE>",
    "NOP",
    "LD",
    "INC",
    "DEC",
    "RLCA",
    "ADD",
    "RRCA",
    "STOP",
    "RLA",
    "JR",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "XOR",
    "OR",
    "CP",
    "POP",
    "JP",
    "PUSH",
    "RET",
    "CB",
    "CALL",
    "RETI",
    "LDH",
    "JPHL",
    "DI",
    "EI",
    "RST",
    "IN_ERR",
    "IN_RLC",
    "IN_RRC",
    "IN_RL",
    "IN_RR",
    "IN_SLA",
    "IN_SRA",
    "IN_SWAP",
    "IN_SRL",
    "IN_BIT",
    "IN_RES",
    "IN_SET"
  };

const char* getInstrName(InstructionType TYPE) 
{ return inst_lookup[TYPE].c_str(); }


// - - - CPU - - - 

FORGE_API void cpuInit()
{
  FORGE_LOG_INFO("Started the emulator");
  cpuCTX.registerFile.programCounter = 0x100;
  cpuCTX.registerFile.accumulator    = 0x1;
}


FORGE_API void cpuTick()
{
  if (!cpuCTX.halted)
  {
    u16 oldPC = cpuCTX.registerFile.programCounter;

    // - - - Fetch instruction
    cpuCTX.currentOpcode     = busRead(cpuCTX.registerFile.programCounter++);
    cpuCTX.memDest           = 0;
    cpuCTX.destIsMemory      = false;
    cpuCTX.currentInst       = &instructions[cpuCTX.currentOpcode];

    if (cpuCTX.currentInst->type == INSTR_NONE)
    {
      std::string message = "Cannot execute instruction : " + std::string(getInstrName(cpuCTX.currentInst->type));
      TODO_COMMENT(message.c_str());
    }

    // - - - fetch the data
    switch (cpuCTX.currentInst->mode)
    {
      // - - - Implied mode : nothing needs to be read
      case ADDR_MODE_IMP   : break;

      // - - - Register : 
      case ADDR_MODE_R     : 
        {
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg1);
          break;
        }

      // - - - From ROM to register 
      case ADDR_MODE_R_D8   :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      // - - - 16 bit registers
      case ADDR_MODE_D16    : 
      case ADDR_MODE_R_D16  : 
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          cpuCTX.readData = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;
          break;
        }

      // - - - from Register(reg2) to memory (reg1)
      case ADDR_MODE_MR_R   :
        {
          cpuCTX.readData     = cpuReadRegister(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;

          // - - - special type with C 
          if (cpuCTX.currentInst->reg1 == REG_C) cpuCTX.memDest |= 0xFF00;
          break;
        }

      // - - - from memory(reg2) to register 
      case ADDR_MODE_R_MR    :
        {
          u16 addr = cpuReadRegister(cpuCTX.currentInst->reg2);

          if (cpuCTX.currentInst->reg1 == REG_C) addr |= 0xFF00;

          cpuCTX.readData = busRead(addr);
          cycles(1);

          break;
        }

      // - - - increment the HL registers
      case ADDR_MODE_R_HLI   :
        {
          cpuCTX.readData = busRead(cpuCTX.currentInst->reg2);
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDR_MODE_R_HLD   :
        {
          cpuCTX.readData = busRead(cpuCTX.currentInst->reg2);
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
          break;
        }
      
      // - - - increment the HL registers
      case ADDR_MODE_HLI_R   :
        {
          cpuCTX.readData     = busRead(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = busRead(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDR_MODE_HLD_R   :
        {
          cpuCTX.readData     = busRead(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = busRead(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
          break;
        }

      case ADDR_MODE_R_A8    :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }
      
      case ADDR_MODE_A8_R    :
        {
          cpuCTX.memDest      = busRead(cpuCTX.registerFile.programCounter) | 0xFF00;
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      case ADDR_MODE_HL_SPR  :
      case ADDR_MODE_D8      :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;
          break;
        }

      case ADDR_MODE_A16_R   :
      case ADDR_MODE_D16_R   : 
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          cpuCTX.memDest = lo | (hi << 8);
          cpuCTX.destIsMemory = true;
          cpuCTX.registerFile.programCounter += 2;
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg2);

          break;
        }

      case ADDR_MODE_MR_D8     :
        {
          cpuCTX.readData = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          cpuCTX.registerFile.programCounter++;

          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;

          break;
        }
      
      case ADDR_MODE_MR    :
        {
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cpuCTX.readData     = busRead(cpuReadRegister(cpuCTX.currentInst->reg1));
          cycles(1);

          break;
        }

      case ADDR_MODE_R_A16 :
        {
          u16 lo = busRead(cpuCTX.registerFile.programCounter);
          cycles(1);
          u16 hi = busRead(cpuCTX.registerFile.programCounter + 1);
          cycles(1);

          u16 addr = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;

          cpuCTX.readData = busRead(addr);
          cycles(1);
          break;
        }

      default : TODO_COMMENT("For now only one addressing mode : Implied")
    }

    // - - - execute the instruction
    FORGE_LOG_INFO(
      "0x%04X : \t\t0x%02X : %-10s \t\t(A : 0x%02X \t BC : 0x%02X%02X \t DE : 0x%02X%02X \t HL : 0x%02X%02X, \t\t FLAGS-  : %c%c%c%c)",
      cpuCTX.registerFile.programCounter - 1,
      cpuCTX.currentOpcode,
      getInstrName(cpuCTX.currentInst->type),
      cpuCTX.registerFile.accumulator,
      cpuCTX.registerFile.b,
      cpuCTX.registerFile.c,
      cpuCTX.registerFile.d,
      cpuCTX.registerFile.e,
      cpuCTX.registerFile.h,
      cpuCTX.registerFile.l,
      cpuCTX.registerFile.flags & (1 << 7) ? 'Z' : '-',
      cpuCTX.registerFile.flags & (1 << 6) ? 'N' : '-',
      cpuCTX.registerFile.flags & (1 << 5) ? 'H' : '-',
      cpuCTX.registerFile.flags & (1 << 4) ? 'C' : '-'
    );

    Processor proc = getInstrProcessor(cpuCTX.currentInst->type);
    FORGE_ASSERT_MESSAGE(proc, "Cannot have a null processor for an instruction");
    proc(&cpuCTX);    
  }
}

FORGE_INLINE u16 reverse(u16 N)
{ return ((N & 0xFF00) >> 8 | ((N & 0x00FF) << 8)); }

u16 cpuReadRegister(RegisterType TYPE)
{
  switch (TYPE) 
  {
    case REG_A : return cpuCTX.registerFile.accumulator;
    case REG_F : return cpuCTX.registerFile.flags;
    case REG_B : return cpuCTX.registerFile.b;
    case REG_C : return cpuCTX.registerFile.c;
    case REG_D : return cpuCTX.registerFile.d;
    case REG_E : return cpuCTX.registerFile.e;
    case REG_H : return cpuCTX.registerFile.h;
    case REG_L : return cpuCTX.registerFile.l;

    case REG_AF : return reverse(*(u16*)&cpuCTX.registerFile.accumulator);
    case REG_BC : return reverse(*(u16*)&cpuCTX.registerFile.b);
    case REG_DE : return reverse(*(u16*)&cpuCTX.registerFile.d);
    case REG_HL : return reverse(*(u16*)&cpuCTX.registerFile.h);

    case REG_PC : return cpuCTX.registerFile.programCounter;
    case REG_SP : return cpuCTX.registerFile.stackPointer;

    default : return 0;
  }
}

void cpuSetRegister(RegisterType TYPE, u16 VAL)
{
  switch (TYPE) 
  {
    case REG_A : cpuCTX.registerFile.accumulator = VAL & 0xFF; return; 
    case REG_F : cpuCTX.registerFile.flags       = VAL & 0xFF; return; 
    case REG_B : cpuCTX.registerFile.b           = VAL & 0xFF; return; 
    case REG_C : cpuCTX.registerFile.c           = VAL & 0xFF; return; 
    case REG_D : cpuCTX.registerFile.d           = VAL & 0xFF; return; 
    case REG_E : cpuCTX.registerFile.e           = VAL & 0xFF; return; 
    case REG_H : cpuCTX.registerFile.h           = VAL & 0xFF; return; 
    case REG_L : cpuCTX.registerFile.l           = VAL & 0xFF; return; 

    case REG_AF : *(u16*)&cpuCTX.registerFile.accumulator = reverse(VAL); return;
    case REG_BC : *(u16*)&cpuCTX.registerFile.b           = reverse(VAL); return;
    case REG_DE : *(u16*)&cpuCTX.registerFile.d           = reverse(VAL); return;
    case REG_HL : *(u16*)&cpuCTX.registerFile.h           = reverse(VAL); return;

    case REG_PC : cpuCTX.registerFile.programCounter = VAL; return;
    case REG_SP : cpuCTX.registerFile.stackPointer   = VAL; return;

    default : return;
  }
}

u8 cpuGetInterrupt()
{ return cpuCTX.interrupt; }

void cpuSetRegister(u8 INTERRPUT)
{ cpuCTX.interrupt = INTERRPUT; }

RegisterFile* cpuGetRegisters()
{ return &cpuCTX.registerFile; }
