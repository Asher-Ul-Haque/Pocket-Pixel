#include <cpu/instruction.h>
#include <string.h>

/**
 * @file instructions.c
 * @brief Opcode -> Instruction metadata tables (unprefixed + CB-prefixed).
*/

#define ILLEGAL ((Instruction){ \
  .type  = IN_ERR, .mode    = AM_IMP, .reg1       = RT_NONE, .reg2 = RT_NONE, .cond = CT_NONE, \
  .bytes = 1,      .mCycles = 1,      .mCyclesAlt = 0, \
  .flags = FLAGPACK_MAKE(FB_KEEP, FB_KEEP, FB_KEEP, FB_KEEP), \
  .param = 0 })

static Instruction opcodes[256];
static Instruction cbOpcodes[256];
static bool initialized = false;


// - - - Small builders - - - 

static inline FlagBehavior fbFromChar(char CHAR)
{
  switch (CHAR)
  {
    case '-': return FB_KEEP;
    case '0': return FB_RESET;
    case '1': return FB_SET;
    case 'Z':
    case 'N':
    case 'H':
    case 'C': return FB_DEPENDS;
    default:  return FB_KEEP;
  }
}


// - - - Convenience for the common fields - - - 

static inline FlagPack fp(char Z, char N, char H, char C)
{
  return FLAGPACK_MAKE(fbFromChar(Z), fbFromChar(N), fbFromChar(H), fbFromChar(C));
}

static inline u8   mcy  (u32 T_CYCLES)              { return (u8)(T_CYCLES / 4u); }
static inline void setOp(u8 OP, Instruction INSTR)  { opcodes[OP] = INSTR; }
static inline void setCB(u8 OP, Instruction INTSTR) { INTSTR.bytes = 2; cbOpcodes[OP] = INTSTR; }

#define OP1(OPC, TYPE, MODE, R1, R2, COND, BYTES, TCYC0, TCYC1_OR0, FLAGS, PARAM) \
  setOp((u8)(OPC), (Instruction){ \
    .type=(TYPE), .mode=(MODE), .reg1=(R1), .reg2=(R2), .cond=(COND), \
    .bytes=(BYTES), .mCycles=mcy((TCYC0)), .mCyclesAlt=mcy((TCYC1_OR0)), \
    .flags=(FLAGS), .param=(PARAM) \
  })

#define OP(OPC, TYPE, MODE, R1, R2, COND, BYTES, TCYC0, FLAGS) \
  OP1((OPC),(TYPE),(MODE),(R1),(R2),(COND),(BYTES),(TCYC0),0,(FLAGS),0)

#define OPALT(OPC, TYPE, MODE, R1, R2, COND, BYTES, TCYC_TAKEN, TCYC_NOTTAKEN, FLAGS) \
  OP1((OPC),(TYPE),(MODE),(R1),(R2),(COND),(BYTES),(TCYC_TAKEN),(TCYC_NOTTAKEN),(FLAGS),0)

#define CB(OPC, TYPE, MODE, R1, R2, BYTES, TCYC0, FLAGS, PARAM) \
  setCB((u8)(OPC), (Instruction){ \
    .type=(TYPE), .mode=(MODE), .reg1=(R1), .reg2=(R2), .cond=CT_NONE, \
    .bytes=(BYTES), .mCycles=mcy((TCYC0)), .mCyclesAlt=0, .flags=(FLAGS), .param=(PARAM) \
  })

static void initTables(void)
{
  if (initialized) return;

  for (u32 i = 0; i < 256; i++)
  {
    opcodes[i]            = ILLEGAL;
    cbOpcodes[i]          = ILLEGAL;
    cbOpcodes[i].bytes    = 2;
  }

  const RegType r8[8] = { RT_B, RT_C, RT_D, RT_E, RT_H, RT_L, RT_HL, RT_A };


  // - - - 0x00 ... 0x3F: Mixed Load, Arithmetic, Control - - - 

  OP(0x00, IN_NOP,  AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 4,  fp('-', '-', '-', '-'));
  OP(0x01, IN_LD,   AM_R_D16, RT_BC,   RT_NONE, CT_NONE, 3, 12, fp('-', '-', '-', '-'));
  OP(0x02, IN_LD,   AM_MR_R,  RT_BC,   RT_A,    CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x03, IN_INC,  AM_R,     RT_BC,   RT_NONE, CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x04, IN_INC,  AM_R,     RT_B,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '0', 'H', '-'));
  OP(0x05, IN_DEC,  AM_R,     RT_B,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '1', 'H', '-'));
  OP(0x06, IN_LD,   AM_R_D8,  RT_B,    RT_NONE, CT_NONE, 2, 8,  fp('-', '-', '-', '-'));
  OP(0x07, IN_RLCA, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 4,  fp('0', '0', '0', 'C'));
  OP(0x08, IN_LD,   AM_A16_R, RT_SP,   RT_NONE, CT_NONE, 3, 20, fp('-', '-', '-', '-'));
  OP(0x09, IN_ADD,  AM_R_R,   RT_HL,   RT_BC,   CT_NONE, 1, 8,  fp('-', '0', 'H', 'C'));
  OP(0x0A, IN_LD,   AM_R_MR,  RT_A,    RT_BC,   CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x0B, IN_DEC,  AM_R,     RT_BC,   RT_NONE, CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x0C, IN_INC,  AM_R,     RT_C,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '0', 'H', '-'));
  OP(0x0D, IN_DEC,  AM_R,     RT_C,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '1', 'H', '-'));
  OP(0x0E, IN_LD,   AM_R_D8,  RT_C,    RT_NONE, CT_NONE, 2, 8,  fp('-', '-', '-', '-'));
  OP(0x0F, IN_RRCA, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 4,  fp('0', '0', '0', 'C'));

  OP(0x10, IN_STOP, AM_D8,    RT_NONE, RT_NONE, CT_NONE, 2, 4,  fp('-', '-', '-', '-'));
  OP(0x11, IN_LD,   AM_R_D16, RT_DE,   RT_NONE, CT_NONE, 3, 12, fp('-', '-', '-', '-'));
  OP(0x12, IN_LD,   AM_MR_R,  RT_DE,   RT_A,    CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x13, IN_INC,  AM_R,     RT_DE,   RT_NONE, CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x14, IN_INC,  AM_R,     RT_D,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '0', 'H', '-'));
  OP(0x15, IN_DEC,  AM_R,     RT_D,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '1', 'H', '-'));
  OP(0x16, IN_LD,   AM_R_D8,  RT_D,    RT_NONE, CT_NONE, 2, 8,  fp('-', '-', '-', '-'));
  OP(0x17, IN_RLA,  AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 4,  fp('0', '0', '0', 'C'));
  OP(0x18, IN_JR,   AM_D8,    RT_NONE, RT_NONE, CT_NONE, 2, 12, fp('-', '-', '-', '-'));
  OP(0x19, IN_ADD,  AM_R_R,   RT_HL,   RT_DE,   CT_NONE, 1, 8,  fp('-', '0', 'H', 'C'));
  OP(0x1A, IN_LD,   AM_R_MR,  RT_A,    RT_DE,   CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x1B, IN_DEC,  AM_R,     RT_DE,   RT_NONE, CT_NONE, 1, 8,  fp('-', '-', '-', '-'));
  OP(0x1C, IN_INC,  AM_R,     RT_E,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '0', 'H', '-'));
  OP(0x1D, IN_DEC,  AM_R,     RT_E,    RT_NONE, CT_NONE, 1, 4,  fp('Z', '1', 'H', '-'));
  OP(0x1E, IN_LD,   AM_R_D8,  RT_E,    RT_NONE, CT_NONE, 2, 8,  fp('-', '-', '-', '-'));
  OP(0x1F, IN_RRA,  AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 4,  fp('0', '0', '0', 'C'));

  OPALT(0x20, IN_JR,  AM_D8,    RT_NONE, RT_NONE, CT_NZ,   2,  12, 8, fp('-', '-', '-', '-'));
  OP(0x21,    IN_LD,  AM_R_D16, RT_HL,   RT_NONE, CT_NONE, 3,  12,    fp('-', '-', '-', '-'));
  OP1(0x22,   IN_LD,  AM_MR_R,  RT_HL,   RT_A,    CT_NONE, 1,  8, 0,  fp('-', '-', '-', '-'), 1);
  OP(0x23,    IN_INC, AM_R,     RT_HL,   RT_NONE, CT_NONE, 1,  8,     fp('-', '-', '-', '-'));
  OP(0x24,    IN_INC, AM_R,     RT_H,    RT_NONE, CT_NONE, 1,  4,     fp('Z', '0', 'H', '-'));
  OP(0x25,    IN_DEC, AM_R,     RT_H,    RT_NONE, CT_NONE, 1,  4,     fp('Z', '1', 'H', '-'));
  OP(0x26,    IN_LD,  AM_R_D8,  RT_H,    RT_NONE, CT_NONE, 2,  8,     fp('-', '-', '-', '-'));
  OP(0x27,    IN_DAA, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1,  4,     fp('Z', '-', '0', 'C'));
  OPALT(0x28, IN_JR,  AM_D8,    RT_NONE, RT_NONE, CT_Z,    2,  12, 8, fp('-', '-', '-', '-'));
  OP(0x29,    IN_ADD, AM_R_R,   RT_HL,   RT_HL,   CT_NONE, 1,  8,     fp('-', '0', 'H', 'C'));
  OP1(0x2A,   IN_LD,  AM_R_MR,  RT_A,    RT_HL,   CT_NONE, 1,  8, 0,  fp('-', '-', '-', '-'), 1);
  OP(0x2B,    IN_DEC, AM_R,     RT_HL,   RT_NONE, CT_NONE, 1,  8,     fp('-', '-', '-', '-'));
  OP(0x2C,    IN_INC, AM_R,     RT_L,    RT_NONE, CT_NONE, 1,  4,     fp('Z', '0', 'H', '-'));
  OP(0x2D,    IN_DEC, AM_R,     RT_L,    RT_NONE, CT_NONE, 1,  4,     fp('Z', '1', 'H', '-'));
  OP(0x2E,    IN_LD,  AM_R_D8,  RT_L,    RT_NONE, CT_NONE, 2,  8,     fp('-', '-', '-', '-'));
  OP(0x2F,    IN_CPL, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1,  4,     fp('-', '1', '1', '-'));

  OPALT(0x30, IN_JR,  AM_D8,    RT_NONE, RT_NONE, CT_NC,   2, 12, 8, fp('-', '-', '-', '-'));
  OP(0x31,    IN_LD,  AM_R_D16, RT_SP,   RT_NONE, CT_NONE, 3, 12,    fp('-', '-', '-', '-'));
  OP1(0x32,   IN_LD,  AM_MR_R,  RT_HL,   RT_A,    CT_NONE, 1,  8, 0, fp('-', '-', '-', '-'), (u8)0xFF);
  OP(0x33,    IN_INC, AM_R,     RT_SP,   RT_NONE, CT_NONE, 1,  8,    fp('-', '-', '-', '-'));
  OP(0x34,    IN_INC, AM_MR_R,  RT_HL,   RT_NONE, CT_NONE, 1, 12,    fp('Z', '0', 'H', '-'));
  OP(0x35,    IN_DEC, AM_MR_R,  RT_HL,   RT_NONE, CT_NONE, 1, 12,    fp('Z', '1', 'H', '-'));
  OP(0x36,    IN_LD,  AM_MR_D8, RT_HL,   RT_NONE, CT_NONE, 2, 12,    fp('-', '-', '-', '-'));
  OP(0x37,    IN_SCF, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1,  4,    fp('-', '0', '0', '1'));
  OPALT(0x38, IN_JR,  AM_D8,    RT_NONE, RT_NONE, CT_C, 2, 12, 8,    fp('-', '-', '-', '-'));
  OP(0x39,    IN_ADD, AM_R_R,   RT_HL,   RT_SP,   CT_NONE, 1,  8,    fp('-', '0', 'H', 'C'));
  OP1(0x3A,   IN_LD,  AM_R_MR,  RT_A,    RT_HL,   CT_NONE, 1,  8, 0, fp('-', '-', '-', '-'), (u8)0xFF);
  OP(0x3B,    IN_DEC, AM_R,     RT_SP,   RT_NONE, CT_NONE, 1,  8,    fp('-', '-', '-', '-'));
  OP(0x3C,    IN_INC, AM_R,     RT_A,    RT_NONE, CT_NONE, 1,  4,    fp('Z', '0', 'H', '-'));
  OP(0x3D,    IN_DEC, AM_R,     RT_A,    RT_NONE, CT_NONE, 1,  4,    fp('Z', '1', 'H', '-'));
  OP(0x3E,    IN_LD,  AM_R_D8,  RT_A,    RT_NONE, CT_NONE, 2,  8,    fp('-', '-', '-', '-'));
  OP(0x3F,    IN_CCF, AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1,  4,    fp('-', '0', '0', 'C'));


  // - - - 0x40 ... 0x7F: 8-bit Load Grid (and HALT) - - -

  for (u32 dst = 0; dst < 8; dst++) 
  {
    for (u32 src = 0; src < 8; src++) 
    {
      u8 opc = (u8)(0x40u + (dst * 8u) + src);
      if (opc == 0x76) 
      {
        OP(0x76, IN_HALT, AM_IMP, RT_NONE, RT_NONE, CT_NONE, 1, 4, fp('-', '-', '-', '-'));
        continue;
      }
      RegType d = r8[dst];
      RegType s = r8[src];
      if (d == RT_HL)      OP(opc, IN_LD, AM_MR_R, RT_HL, s,     CT_NONE, 1, 8, fp('-', '-', '-', '-'));
      else if (s == RT_HL) OP(opc, IN_LD, AM_R_MR, d,     RT_HL, CT_NONE, 1, 8, fp('-', '-', '-', '-'));
      else                 OP(opc, IN_LD, AM_R_R,  d,     s,     CT_NONE, 1, 4, fp('-', '-', '-', '-'));
    }
  }


  // - - - 0x80 ... 0xBF: ALU Grid (ADD, ADC, SUB, SBC, AND, XOR, OR, CP) - - - 

  InstructionType aluTypes[8] = { IN_ADD, IN_ADC, IN_SUB, IN_SBC, IN_AND, IN_XOR, IN_OR, IN_CP };
  FlagPack        aluFlags[8] = 
    {
      fp('Z','0','H','C'), fp('Z','0','H','C'), fp('Z','1','H','C'), fp('Z','1','H','C'),
      fp('Z','0','1','0'), fp('Z','0','0','0'), fp('Z','0','0','0'), fp('Z','1','H','C')
    };

  for (u32 grp = 0; grp < 8; grp++) 
  {
    for (u32 i = 0; i < 8; i++) 
    {
      u8          opc  = (u8)(0x80u + (grp * 8u) + i);
      RegType     s    = r8[i];
      u32         cyc  = (s == RT_HL) ? 8 : 4;
      AddressMode mode = (s == RT_HL) ? AM_R_MR : AM_R_R;
      FlagPack    f    = aluFlags[grp];

      if (opc == 0x97 || opc == 0xBF) f = fp('1','1','0','0');
      if (opc == 0x9F)                f = fp('Z','1','H','-');
      if (opc == 0xAF)                f = fp('1','0','0','0');

      OP(opc, aluTypes[grp], mode, RT_A, s, CT_NONE, 1, cyc, f);
    }
  }


  // - - - 0xC0 ... 0xFF: High Opcodes - - -

  OPALT(0xC0, IN_RET,  AM_IMP,  RT_NONE, RT_NONE, CT_NZ,   1, 20, 8,  fp('-', '-', '-', '-'));
  OP   (0xC1, IN_POP,  AM_R,    RT_BC,   RT_NONE, CT_NONE, 1, 12,     fp('-', '-', '-', '-'));
  OPALT(0xC2, IN_JP,   AM_D16,  RT_NONE, RT_NONE, CT_NZ,   3, 16, 12, fp('-', '-', '-', '-'));
  OP   (0xC3, IN_JP,   AM_D16,  RT_NONE, RT_NONE, CT_NONE, 3, 16,     fp('-', '-', '-', '-'));
  OPALT(0xC4, IN_CALL, AM_D16,  RT_NONE, RT_NONE, CT_NZ,   3, 24, 12, fp('-', '-', '-', '-'));
  OP   (0xC5, IN_PUSH, AM_R,    RT_BC,   RT_NONE, CT_NONE, 1, 16,     fp('-', '-', '-', '-'));
  OP   (0xC6, IN_ADD,  AM_R_D8, RT_A,    RT_NONE, CT_NONE, 2,  8,     fp('Z', '0', 'H', 'C'));
  OP1  (0xC7, IN_RST,  AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16, 0,  fp('-', '-', '-', '-'), 0x00);
  OPALT(0xC8, IN_RET,  AM_IMP,  RT_NONE, RT_NONE, CT_Z,    1, 20, 8,  fp('-', '-', '-', '-'));
  OP   (0xC9, IN_RET,  AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16,     fp('-', '-', '-', '-'));
  OPALT(0xCA, IN_JP,   AM_D16,  RT_NONE, RT_NONE, CT_Z,    3, 16, 12, fp('-', '-', '-', '-'));
  OP   (0xCB, IN_CB,   AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1,  4,     fp('-', '-', '-', '-'));
  OPALT(0xCC, IN_CALL, AM_D16,  RT_NONE, RT_NONE, CT_Z,    3, 24, 12, fp('-', '-', '-', '-'));
  OP   (0xCD, IN_CALL, AM_D16,  RT_NONE, RT_NONE, CT_NONE, 3, 24,     fp('-', '-', '-', '-'));
  OP   (0xCE, IN_ADC,  AM_R_D8, RT_A,    RT_NONE, CT_NONE, 2,  8,     fp('Z', '0', 'H', 'C'));
  OP1  (0xCF, IN_RST,  AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16, 0,  fp('-', '-', '-', '-'), 0x08);

  OPALT(0xD0, IN_RET,  AM_IMP,  RT_NONE, RT_NONE, CT_NC,   1, 20,  8, fp('-', '-', '-', '-'));
  OP   (0xD1, IN_POP,  AM_R,    RT_DE,   RT_NONE, CT_NONE, 1, 12,     fp('-', '-', '-', '-'));
  OPALT(0xD2, IN_JP,   AM_D16,  RT_NONE, RT_NONE, CT_NC,   3, 16, 12, fp('-', '-', '-', '-'));
  OPALT(0xD4, IN_CALL, AM_D16,  RT_NONE, RT_NONE, CT_NC,   3, 24, 12, fp('-', '-', '-', '-'));
  OP   (0xD5, IN_PUSH, AM_R,    RT_DE,   RT_NONE, CT_NONE, 1, 16,     fp('-', '-', '-', '-'));
  OP   (0xD6, IN_SUB,  AM_R_D8, RT_A,    RT_NONE, CT_NONE, 2,  8,     fp('Z', '1', 'H', 'C'));
  OP1  (0xD7, IN_RST,  AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16,  0, fp('-', '-', '-', '-'), 0x10);
  OPALT(0xD8, IN_RET,  AM_IMP,  RT_NONE, RT_NONE, CT_C,    1, 20,  8, fp('-', '-', '-', '-'));
  OP   (0xD9, IN_RETI, AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16,     fp('-', '-', '-', '-'));
  OPALT(0xDA, IN_JP,   AM_D16,  RT_NONE, RT_NONE, CT_C,    3, 16, 12, fp('-', '-', '-', '-'));
  OPALT(0xDC, IN_CALL, AM_D16,  RT_NONE, RT_NONE, CT_C,    3, 24, 12, fp('-', '-', '-', '-'));
  OP   (0xDE, IN_SBC,  AM_R_D8, RT_A,    RT_NONE, CT_NONE, 2,  8,     fp('Z', '1', 'H', 'C'));
  OP1  (0xDF, IN_RST,  AM_IMP,  RT_NONE, RT_NONE, CT_NONE, 1, 16, 0,  fp('-', '-', '-', '-'), 0x18);

  OP(0xE0,    IN_LDH,  AM_A8_R,  RT_A,    RT_NONE, CT_NONE, 2, 12,    fp('-', '-', '-', '-'));
  OP(0xE1,    IN_POP,  AM_R,     RT_HL,   RT_NONE, CT_NONE, 1, 12,    fp('-', '-', '-', '-'));
  OP(0xE2,    IN_LDH,  AM_MR_C,  RT_A,    RT_NONE, CT_NONE, 1,  8,    fp('-', '-', '-', '-'));
  OP(0xE5,    IN_PUSH, AM_R,     RT_HL,   RT_NONE, CT_NONE, 1, 16,    fp('-', '-', '-', '-'));
  OP(0xE6,    IN_AND,  AM_R_D8,  RT_A,    RT_NONE, CT_NONE, 2,  8,    fp('Z', '0', '1', '0'));
  OP1(0xE7,   IN_RST,  AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 16, 0, fp('-', '-', '-', '-'), 0x20);
  OP1(0xE8,   IN_ADD,  AM_R_D8,  RT_SP,   RT_NONE, CT_NONE, 2, 16, 0, fp('0', '0', 'H', 'C'), 1);
  OP(0xE9,    IN_JPHL, AM_IMP,   RT_HL,   RT_NONE, CT_NONE, 1,  4,    fp('-', '-', '-', '-'));
  OP(0xEA,    IN_LD,   AM_A16_R, RT_A,    RT_NONE, CT_NONE, 3, 16,    fp('-', '-', '-', '-'));
  OP(0xEE,    IN_XOR,  AM_R_D8,  RT_A,    RT_NONE, CT_NONE, 2,  8,    fp('Z', '0', '0', '0'));
  OP1(0xEF,   IN_RST,  AM_IMP,   RT_NONE, RT_NONE, CT_NONE, 1, 16, 0, fp('-', '-', '-', '-'), 0x28);

  OP(0xF0,    IN_LDH,  AM_R_A8,   RT_A,    RT_NONE, CT_NONE, 2, 12,    fp('-', '-', '-', '-'));
  OP(0xF1,    IN_POP,  AM_R,      RT_AF,   RT_NONE, CT_NONE, 1, 12,    fp('Z', 'N', 'H', 'C'));
  OP(0xF2,    IN_LDH,  AM_R_MR_C, RT_A,    RT_NONE, CT_NONE, 1,  8,    fp('-', '-', '-', '-'));
  OP(0xF3,    IN_DI,   AM_IMP,    RT_NONE, RT_NONE, CT_NONE, 1,  4,    fp('-', '-', '-', '-'));
  OP(0xF5,    IN_PUSH, AM_R,      RT_AF,   RT_NONE, CT_NONE, 1, 16,    fp('-', '-', '-', '-'));
  OP(0xF6,    IN_OR,   AM_R_D8,   RT_A,    RT_NONE, CT_NONE, 2,  8,    fp('Z', '0', '0', '0'));
  OP1(0xF7,   IN_RST,  AM_IMP,    RT_NONE, RT_NONE, CT_NONE, 1, 16, 0, fp('-', '-', '-', '-'), 0x30);
  OP1(0xF8,   IN_LD,   AM_R_R,    RT_HL,   RT_SP,   CT_NONE, 2, 12, 0, fp('0', '0', 'H', 'C'), 1);
  OP(0xF9,    IN_LD,   AM_R_R,    RT_SP,   RT_HL,   CT_NONE, 1,  8,    fp('-', '-', '-', '-'));
  OP(0xFA,    IN_LD,   AM_R_A16,  RT_A,    RT_NONE, CT_NONE, 3, 16,    fp('-', '-', '-', '-'));
  OP(0xFB,    IN_EI,   AM_IMP,    RT_NONE, RT_NONE, CT_NONE, 1,  4,    fp('-', '-', '-', '-'));
  OP(0xFE,    IN_CP,   AM_R_D8,   RT_A,    RT_NONE, CT_NONE, 2,  8,    fp('Z', '1', 'H', 'C'));
  OP1(0xFF,   IN_RST,  AM_IMP,    RT_NONE, RT_NONE, CT_NONE, 1, 16, 0, fp('-', '-', '-', '-'), 0x38);


  // - - - CB-Prefixed Instructions - - - 

  InstructionType cbArithmetic[8] = { IN_RLC, IN_RRC, IN_RL, IN_RR, IN_SLA, IN_SRA, IN_SWAP, IN_SRL };
  for (u32 grp = 0; grp < 8; grp++) 
  {
    for (u32 i = 0; i < 8; i++) 
    {
      u8          opc     = (u8)(grp * 8u + i);
      RegType     target  = r8[i];
      u32         cyc     = (target == RT_HL) ? 16 : 8;
      FlagPack    f       = (cbArithmetic[grp] == IN_SWAP) ? fp('Z','0','0','0') : fp('Z','0','0','C');
      AddressMode mode    = (target == RT_HL) ? AM_MR_R : AM_R;
      CB(opc, cbArithmetic[grp], mode, target, RT_NONE, 2, cyc, f, 0);
    }
  }


  // - - - BIT, RES, SET CB Grids - - - 

  for (u32 bit = 0; bit < 8; bit++) 
  {
    for (u32 i = 0; i < 8; i++) 
    {
      RegType     target = r8[i];
      AddressMode mode   = (target == RT_HL) ? AM_MR_R : AM_R;

      CB(0x40 + bit*8 + i, IN_BIT, mode, target, RT_NONE, 2, (target == RT_HL ? 12 : 8), fp('Z','0','1','-'), bit);
      CB(0x80 + bit*8 + i, IN_RES, mode, target, RT_NONE, 2, (target == RT_HL ? 16 : 8), fp('-','-','-','-'), bit);
      CB(0xC0 + bit*8 + i, IN_SET, mode, target, RT_NONE, 2, (target == RT_HL ? 16 : 8), fp('-','-','-','-'), bit);
    }
  }

  initialized = true;
}

const Instruction* instructionGetByOpcode(u8 OPCODE)
{
  initTables();
  return &opcodes[OPCODE];
}

const Instruction* instructionGetByCBOpcode(u8 CB_OPCODE)
{
  initTables();
  return &cbOpcodes[CB_OPCODE];
}

const char* instructionGetName(InstructionType TYPE)
{
  switch (TYPE)
  {
    case IN_NONE: return "NONE";
    case IN_NOP:  return "NOP";
    case IN_LD:   return "LD";
    case IN_LDH:  return "LDH";
    case IN_INC:  return "INC";
    case IN_DEC:  return "DEC";

    case IN_RLCA: return "RLCA";
    case IN_RRCA: return "RRCA";
    case IN_RLA:  return "RLA";
    case IN_RRA:  return "RRA";

    case IN_DAA:  return "DAA";
    case IN_CPL:  return "CPL";
    case IN_SCF:  return "SCF";
    case IN_CCF:  return "CCF";

    case IN_STOP: return "STOP";
    case IN_HALT: return "HALT";
    case IN_DI:   return "DI";
    case IN_EI:   return "EI";

    case IN_JP:   return "JP";
    case IN_JPHL: return "JP";
    case IN_JR:   return "JR";
    case IN_CALL: return "CALL";
    case IN_RET:  return "RET";
    case IN_RETI: return "RETI";
    case IN_RST:  return "RST";

    case IN_PUSH: return "PUSH";
    case IN_POP:  return "POP";

    case IN_ADD:  return "ADD";
    case IN_ADC:  return "ADC";
    case IN_SUB:  return "SUB";
    case IN_SBC:  return "SBC";
    case IN_AND:  return "AND";
    case IN_XOR:  return "XOR";
    case IN_OR:   return "OR";
    case IN_CP:   return "CP";

    case IN_CB:   return "CB";

    case IN_RLC:  return "RLC";
    case IN_RRC:  return "RRC";
    case IN_RL:   return "RL";
    case IN_RR:   return "RR";
    case IN_SLA:  return "SLA";
    case IN_SRA:  return "SRA";
    case IN_SWAP: return "SWAP";
    case IN_SRL:  return "SRL";
    case IN_BIT:  return "BIT";
    case IN_RES:  return "RES";
    case IN_SET:  return "SET";

    case IN_ERR:  return "ERR";
    default:      return "UNK";
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
