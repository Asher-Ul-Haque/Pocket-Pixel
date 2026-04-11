#include <cpu/cpu.h>
#include <cpu/instructions.h>
#include <cpu/cpuOpcodeHandlers.h> 
#include <bus.h>

/**
 * @file cpu_exec.c
 * @brief Opcode dispatch tables.
*/

void opILLEGAL(u8 OPCODE)
{
  CpuContext* ctx = cpuGetContext();
  u16         pc  = ctx->registers.programCounter;
  u16 opcodeAddr  = (pc > 0) ? (u16)(pc - 1) : pc;

  FORGE_LOG_ERROR("[CPU] : Illegal opcode 0x%02X at PC=0x%04X", OPCODE, opcodeAddr);
  FORGE_ASSERT_DEBUG(false, "Illegal opcode executed");
  ctx->halted = true;
}

void opNOP(u8 OPCODE)
{
  (void)OPCODE;
}

static CpuOpcodeHandler opcodeHandlers   [256] = {0};
static CpuOpcodeHandler opcodeHandlersCB [256] = {0};
static Instruction      instructions     [256] = {0};
static Instruction      instructionsCB   [256] = {0};

static const char*      instructionTypeNames[]  =
  {
    "<NONE>", ///< IN_NONE  
    "NOP",    ///< IN_NOP
    "LD",     ///< IN_LD 
    "INC",    ///< IN_INC
    "DEC",    ///< IN_DEC 
    "RLCA",   ///< IN_RLCA 
    "ADD",    ///< IN_ADD 
    "RRCA",   ///< IN_RRCA 
    "STOP",   ///< IN_STOP 
    "RLA",    ///< IN_RLA 
    "JR",     ///< IN_JR 
    "RRA",    ///< IN_RRA 
    "DAA",    ///< IN_DAA 
    "CPL",    ///< IN_CPL 
    "SCF",    ///< IN_SCF 
    "CCF",    ///< IN_CCF 
    "HALT",   ///< IN_HALT 
    "ADC",    ///< IN_ADC 
    "SUB",    ///< IN_SUB 
    "SBC",    ///< IN_SBC 
    "AND",    ///< IN_AND 
    "XOR",    ///< IN_XOR 
    "OR",     ///< IN_OR 
    "CP",     ///< IN_CP 
    "POP",    ///< IN_POP 
    "JP",     ///< IN_JP 
    "PUSH",   ///< IN_PUSH 
    "RET",    ///< IN_RET 
    "CB",     ///< IN_CB 
    "CALL",   ///< IN_CALL 
    "RETI",   ///< IN_RETI 
    "LDH",    ///< IN_LDH 
    "JP (HL)",///< IN_JPHL 
    "DI",     ///< IN_DI 
    "EI",     ///< IN_EI 
    "RST",    ///< IN_RST 
    "ERR",    ///< IN_ERR 
    "RLC",    ///< IN_RLC 
    "RRC",    ///< IN_RRC 
    "RL",     ///< IN_RL 
    "RR",     ///< IN_RR 
    "SLA",    ///< IN_SLA 
    "SRA",    ///< IN_SRA 
    "SWAP",   ///< IN_SWAP 
    "SRL",    ///< IN_SRL 
    "BIT",    ///< IN_BIT 
    "RES",    ///< IN_RES 
    "SET"     ///< IN_SET 
  };

static const char*      regNames[]              =
  {
    "<NONE>", ///< RT_NONE
    "A",      ///< RT_A
    "B",      ///< RT_B 
    "C",      ///< RT_C 
    "D",      ///< RT_D 
    "E",      ///< RT_E 
    "H",      ///< RT_H 
    "L",      ///< RT_L 
    "AF",     ///< RT_AF 
    "BC",     ///< RT_BC 
    "DE",     ///< RT_DE 
    "HL",     ///< RT_HL 
    "SP",     ///< RT_SP 
    "PC"      ///< RT_PC 
  };

static Instruction makeErr(void)
{
  Instruction i;
  memset(&i, 0, sizeof(Instruction));
  i.type        = IN_ERR;
  i.mode        = AM_IMP;
  i.reg1        = RT_NONE;
  i.reg2        = RT_NONE;
  i.cond        = CT_NONE;
  i.param       = 0;
  i.mCycles     = 1;
  i.mCyclesAlt  = 1;
  return i;
}

static Instruction makeImp(InstructionType TYPE, u8 M)
{
  Instruction i = makeErr();
  i.type        = TYPE;
  i.mCycles     = M;
  i.mCyclesAlt  = M;
  return i;
}

static Instruction makeR(InstructionType TYPE, RegType REG_TYPE, u8 M)
{
  Instruction i = makeErr();
  i.type        = TYPE;
  i.mode        = AM_R;
  i.reg1        = REG_TYPE;
  i.mCycles     = M;
  i.mCyclesAlt  = M;
  return i;
}

static void initTablesOnce(void)
{
  // - - - TODO: fill the tables

  static bool inited = false;
  if (inited) return;
  inited = true;

  Instruction err = makeErr();
  for (u32 i = 0; i < 256; ++i) instructions[i]     = err;
  for (u32 i = 0; i < 256; ++i) instructionsCB[i]   = err;
  for (u32 i = 0; i < 256; ++i) opcodeHandlers[i]   = opILLEGAL; 
  for (u32 i = 0; i < 256; ++i) opcodeHandlersCB[i] = opILLEGAL; 

  instructions[0x00] = makeImp(IN_NOP, 1);
  instructions[0x10] = makeImp(IN_STOP, 1);
  instructions[0x76] = makeImp(IN_HALT, 1);
  instructions[0xF3] = makeImp(IN_DI, 1);
  instructions[0xFB] = makeImp(IN_EI, 1);

  instructionsCB[0xCB] = makeImp(IN_CB, 1);
  instructionsCB[0x11] = makeR(IN_RLC, RT_C, 2);


  // - - - JP / JR - - - 
 
  instructions[0xC3] = makeImp(IN_JP, 4);  instructions[0xC3].mode = AM_D16; opcodeHandlers[0xC3] = opJP_a16;
  instructions[0xE9] = makeImp(IN_JPHL,1); instructions[0xE9].mode = AM_IMP; opcodeHandlers[0xE9] = opJP_HL;

  instructions[0x18] = makeImp(IN_JR, 3);  instructions[0x18].mode = AM_D8;  opcodeHandlers[0x18] = opJR_r8;
  instructions[0x20] = makeImp(IN_JR, 2);  instructions[0x20].mode = AM_D8;  instructions[0x20].cond = CT_NZ; opcodeHandlers[0x20] = opJR_cc_r8;
  instructions[0x28] = makeImp(IN_JR, 2);  instructions[0x28].mode = AM_D8;  instructions[0x28].cond = CT_Z;  opcodeHandlers[0x28] = opJR_cc_r8;
  instructions[0x30] = makeImp(IN_JR, 2);  instructions[0x30].mode = AM_D8;  instructions[0x30].cond = CT_NC; opcodeHandlers[0x30] = opJR_cc_r8;
  instructions[0x38] = makeImp(IN_JR, 2);  instructions[0x38].mode = AM_D8;  instructions[0x38].cond = CT_C;  opcodeHandlers[0x38] = opJR_cc_r8;


  // - - - CALL - - - 

  instructions[0xCD] = makeImp(IN_CALL, 6); instructions[0xCD].mode = AM_D16; opcodeHandlers[0xCD] = opCALL_a16;

  instructions[0xC4] = makeImp(IN_CALL, 3); instructions[0xC4].mode = AM_D16; instructions[0xC4].cond = CT_NZ; opcodeHandlers[0xC4] = opCALL_cc_a16;
  instructions[0xCC] = makeImp(IN_CALL, 3); instructions[0xCC].mode = AM_D16; instructions[0xCC].cond = CT_Z;  opcodeHandlers[0xCC] = opCALL_cc_a16;
  instructions[0xD4] = makeImp(IN_CALL, 3); instructions[0xD4].mode = AM_D16; instructions[0xD4].cond = CT_NC; opcodeHandlers[0xD4] = opCALL_cc_a16;
  instructions[0xDC] = makeImp(IN_CALL, 3); instructions[0xDC].mode = AM_D16; instructions[0xDC].cond = CT_C;  opcodeHandlers[0xDC] = opCALL_cc_a16;


  // - - - RET / RETI - - - 

  instructions[0xC9] = makeImp(IN_RET, 4); instructions[0xC9].mode = AM_IMP; opcodeHandlers[0xC9] = opRET;
  instructions[0xD9] = makeImp(IN_RETI,4); instructions[0xD9].mode = AM_IMP; opcodeHandlers[0xD9] = opRETI;

  instructions[0xC0] = makeImp(IN_RET, 2); instructions[0xC0].mode = AM_IMP; instructions[0xC0].cond = CT_NZ; opcodeHandlers[0xC0] = opRET_cc;
  instructions[0xC8] = makeImp(IN_RET, 2); instructions[0xC8].mode = AM_IMP; instructions[0xC8].cond = CT_Z;  opcodeHandlers[0xC8] = opRET_cc;
  instructions[0xD0] = makeImp(IN_RET, 2); instructions[0xD0].mode = AM_IMP; instructions[0xD0].cond = CT_NC; opcodeHandlers[0xD0] = opRET_cc;
  instructions[0xD8] = makeImp(IN_RET, 2); instructions[0xD8].mode = AM_IMP; instructions[0xD8].cond = CT_C;  opcodeHandlers[0xD8] = opRET_cc;


  // - - - RST - - - -
 
  instructions[0xC7] = makeImp(IN_RST, 4); opcodeHandlers[0xC7] = opRST;
  instructions[0xCF] = makeImp(IN_RST, 4); opcodeHandlers[0xCF] = opRST;
  instructions[0xD7] = makeImp(IN_RST, 4); opcodeHandlers[0xD7] = opRST;
  instructions[0xDF] = makeImp(IN_RST, 4); opcodeHandlers[0xDF] = opRST;
  instructions[0xE7] = makeImp(IN_RST, 4); opcodeHandlers[0xE7] = opRST;
  instructions[0xEF] = makeImp(IN_RST, 4); opcodeHandlers[0xEF] = opRST;
  instructions[0xF7] = makeImp(IN_RST, 4); opcodeHandlers[0xF7] = opRST;
  instructions[0xFF] = makeImp(IN_RST, 4); opcodeHandlers[0xFF] = opRST;


  // - - - CPU control - - - 
 
  opcodeHandlers[0xF3] = opDI;
  opcodeHandlers[0xFB] = opEI;
  opcodeHandlers[0x76] = opHALT;
  opcodeHandlers[0x10] = opSTOP;
  opcodeHandlers[0x00] = opNOP;
}

CpuOpcodeHandler cpuGetOpcodeHandler(u8 OPCODE)
{
  initTablesOnce();
  return opcodeHandlers[OPCODE];
}

CpuOpcodeHandler cpuGetCBOpcodeHandler(u8 CB_OPCODE)
{
  initTablesOnce();
  return opcodeHandlersCB[CB_OPCODE];
}

const Instruction* instructionGetByOpcode(u8 OPCODE)
{
  initTablesOnce();
  return &instructions[OPCODE];
}

const Instruction* instructionGetByCBOpcode(u8 CB_OPCODE)
{
  initTablesOnce();
  return &instructionsCB[CB_OPCODE];
}

const char* instructionGetName(InstructionType TYPE)
{
  u32 idx   = (u32)TYPE;
  u32 count = (u32)(sizeof(instructionTypeNames) / sizeof(instructionTypeNames[0]));

  if (idx >= count) return "<BAD_INSTRUCTION_TYPE>";

  return instructionTypeNames[idx] ? instructionTypeNames[idx] : "<UNNAMED>";
}

const char* instructionGetRegName(RegType REG)
{
  u32 idx   = (u32)REG;
  u32 count = (u32)(sizeof(regNames) / sizeof(regNames[0]));

  if (idx >= count) return "<BAD_REG>";
  return regNames[idx] ? regNames[idx] : "<UNNAMED_REG>";
}

void cpuExecDecoded(void)
{
  initTablesOnce();

  const CpuContext* ctx     = cpuGetContext();
  CpuOpcodeHandler  handler = NULL;

  if (ctx->isCB) handler = cpuGetCBOpcodeHandler(ctx->cbOpcode);
  else           handler = cpuGetOpcodeHandler(ctx->currentOpcode);

  FORGE_ASSERT_DEBUG(handler != NULL, "Opcode handler must not be NULL");
  handler(ctx->isCB ? ctx->cbOpcode : ctx->currentOpcode);
}
