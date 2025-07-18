#include "../include/cpu.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/debugger.h"
#include "../include/instruction.h"
#include "../../../GameBoyCore.h"
#include "../../../ForgeLibrary/include/asserts.h"
#include "../../../ForgeLibrary/include/logger.h"
#include "../include/timer.h"

static CPUContext   cpuCTX    = {0};


// - - - CPU - - - 

FORGE_API void cpuInit()
{
  FORGE_LOG_INFO("Started the emulator");
  cpuCTX.registerFile.programCounter        = 0x100;
  cpuCTX.registerFile.accumulator           = 0x1;
  cpuCTX.interrupt                          = 0;
  cpuCTX.interruptFlags                     = 0;
  cpuCTX.interruptMasterEnabled             = false;
  cpuCTX.enableIME                          = false;
  timerGetContext()->div                    = 0xABCC;
  *((i16*)&cpuCTX.registerFile.accumulator) = 0xB001;
  *((i16*)&cpuCTX.registerFile.b)           = 0x1300;
  *((i16*)&cpuCTX.registerFile.d)           = 0xD800;
  *((i16*)&cpuCTX.registerFile.h)           = 0x4D01;
}


FORGE_API void cpuTick()
{
  if (!cpuCTX.halted)
  {
    u16 oldPC = cpuCTX.registerFile.programCounter;

    // - - - Fetch instruction
    cpuCTX.currentOpcode     = busRead(cpuCTX.registerFile.programCounter++);
    cpuCTX.currentInst       = getInstrByOpcode(cpuCTX.currentOpcode);
    cycles(1);
    cpuCTX.memDest           = 0;
    cpuCTX.destIsMemory      = false;

    FORGE_ASSERT(cpuCTX.currentInst);
    FORGE_ASSERT(cpuCTX.currentInst->type != INSTR_NONE);

    // - - - fetch the data
    switch (cpuCTX.currentInst->mode)
    {
      // - - - Implied mode : nothing needs to be read
      case ADDR_MODE_IMP    : break;

      // - - - Register : 
      case ADDR_MODE_R      : 
        {
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg1);
          break;
        }

      case ADDR_MODE_R_R    :
        {
          cpuCTX.readData = cpuReadRegister(cpuCTX.currentInst->reg2);
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

          cpuCTX.readData                     = lo | (hi << 8);
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
          cpuCTX.readData = busRead(cpuReadRegister(cpuCTX.currentInst->reg2));
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDR_MODE_R_HLD   :
        {
          cpuCTX.readData = busRead(cpuReadRegister(cpuCTX.currentInst->reg2));
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) - 1);
          break;
        }
      
      // - - - increment the HL registers
      case ADDR_MODE_HLI_R   :
        {
          cpuCTX.readData     = cpuReadRegister(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
          cpuCTX.destIsMemory = true;
          cycles(1);
          cpuSetRegister(REG_HL, cpuReadRegister(REG_HL) + 1);
          break;
        }
      
      // - - - decrement the HL registers
      case ADDR_MODE_HLD_R   :
        {
          cpuCTX.readData     = cpuReadRegister(cpuCTX.currentInst->reg2);
          cpuCTX.memDest      = cpuReadRegister(cpuCTX.currentInst->reg1);
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

          cpuCTX.memDest                      = lo | (hi << 8);
          cpuCTX.destIsMemory                 = true;
          cpuCTX.registerFile.programCounter += 2;
          cpuCTX.readData                     = cpuReadRegister(cpuCTX.currentInst->reg2);

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

          u16 addr                            = lo | (hi << 8);
          cpuCTX.registerFile.programCounter += 2;
          cpuCTX.readData                     = busRead(addr);
          cycles(1);
          break;
        }

      default :
      {
        FORGE_LOG_FATAL("Failed to address : %d", cpuCTX.currentInst->mode);
        TODO_COMMENT("For now only one addressing mode : Implied")
      }
    }

    // - - - execute the instruction
    char instruction[1600];
    getInstrStr(cpuCTX.currentInst, cpuCTX.readData, oldPC, instruction);
    FORGE_LOG_INFO(
      "0x%08X - 0x%04X: \t\t0x%02X : %-20s (A : 0x%02X \t BC : 0x%02X%02X \t DE : 0x%02X%02X \t HL : 0x%02X%02X, \t FLAGS : %c%c%c%c)",
      getContext()->ticks++,
      oldPC,
      cpuCTX.currentOpcode,
      instruction,
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
    debuggerUpdate();
    debuggerPrint();

    Processor proc = getInstrProcessor(cpuCTX.currentInst->type);
    FORGE_LOG_INFO("CAUSES ERRO : %d %s", cpuCTX.currentInst->type, instruction);
    FORGE_ASSERT_MESSAGE(proc, "Cannot have a null processor for an instruction");
    proc(&cpuCTX);    
  }
  else 
  {
    cycles(1);
    if (cpuCTX.interruptFlags)    cpuCTX.halted = false;
  }

  if (cpuCTX.interruptMasterEnabled)
  {
    cpuHandleInterrupts(&cpuCTX);
    cpuCTX.enableIME = false;
  }

  if (cpuCTX.enableIME)    cpuCTX.interruptMasterEnabled = true;
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

u8 cpuReadRegister8(RegisterType TYPE)
{
  switch (TYPE)
  {
    case REG_A  : return cpuCTX.registerFile.accumulator;
    case REG_F  : return cpuCTX.registerFile.flags;
    case REG_B  : return cpuCTX.registerFile.b;
    case REG_C  : return cpuCTX.registerFile.c;
    case REG_D  : return cpuCTX.registerFile.d;
    case REG_E  : return cpuCTX.registerFile.e;
    case REG_H  : return cpuCTX.registerFile.h;
    case REG_L  : return cpuCTX.registerFile.l;
    case REG_HL : return busRead(cpuReadRegister(REG_HL)); 
    default :
      {
        FORGE_LOG_FATAL("Invalid 8 bit Register : %d\n", TYPE);
        FORGE_ASSERT(false);
        return 0;
      }
  }
}

void cpuSetRegister8(RegisterType TYPE, u8 VAL)
{
  switch (TYPE)
  {
    case REG_A  : cpuCTX.registerFile.accumulator = VAL & 0xFF; break;
    case REG_F  : cpuCTX.registerFile.flags       = VAL & 0xFF; break;
    case REG_B  : cpuCTX.registerFile.b           = VAL & 0xFF; break;
    case REG_C  : cpuCTX.registerFile.c           = VAL & 0xFF; break;
    case REG_D  : cpuCTX.registerFile.d           = VAL & 0xFF; break;
    case REG_E  : cpuCTX.registerFile.e           = VAL & 0xFF; break;
    case REG_H  : cpuCTX.registerFile.h           = VAL & 0xFF; break;
    case REG_L  : cpuCTX.registerFile.l           = VAL & 0xFF; break;
    case REG_HL : busWrite(cpuReadRegister(REG_HL), VAL);       break;
    default : 
      {
        FORGE_LOG_FATAL("Invalid 8 bit Register : %d\n", TYPE);
        FORGE_ASSERT(false);
      }
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

void cpuSetInterrupt(u8 INTERRUPT)
{ cpuCTX.interrupt = INTERRUPT; }

RegisterFile* cpuGetRegisters()
{ return &cpuCTX.registerFile; }

void cpuRequestInterrupt(InterruptType TYPE)
{ cpuCTX.interruptFlags |= TYPE; }
