#include "cpu.h"
#include "../../GameBoyCore.h"
#include "../../ForgeLibrary/include/asserts.h"


// - - - Helpers - - -


void cpuSetFlags(CPUContext* CTX, char Z, char N, char H, char C)
{
  if (Z != -1)
  {
    CTX->registerFile.flags &= ~(1 << 7);
    CTX->registerFile.flags |= (Z & 1) << 7;
  }
  if (N != -1)
  {
    CTX->registerFile.flags &= ~(1 << 6);
    CTX->registerFile.flags |= (N & 1) << 6;
  }
  if (H != -1)
  {
    CTX->registerFile.flags &= ~(1 << 5);
    CTX->registerFile.flags |= (H & 1) << 5;
  }
  if (C != -1)
  {
    CTX->registerFile.flags &= ~(1 << 4);
    CTX->registerFile.flags |= (C & 1) << 4;
  }
}


// - - - Instruction Processors - - - 

// - - - None : Means not implemented yet
static void procNone(CPUContext* CTX) {}

// - - - Load instruction 
static void procLoad(CPUContext* CTX)
{ TODO }


// - - - XOR, XOR accumulator with whatever is read 
static void procXOR(CPUContext* CTX)
{ 
  CTX->registerFile.accumulator ^= CTX->readData & 0xFF;
  cpuSetFlags(CTX, CTX->registerFile.accumulator, 0, 0, 0);
}

// - - - disable interruprts
static void procDI(CPUContext* CTX)
{ CTX->interruptMasterEnabled = false; }


// - - - jump insturction
static void procJump(CPUContext* CTX)
{
  bool checkCondition = true;

  bool z =  ((CTX->registerFile.flags >> 7) & 1);
  bool c =  ((CTX->registerFile.flags >> 4) & 1);

  switch (CTX->currentInst->cond) 
  {
    case CONDITIONS_NONE : { checkCondition = true; break; }
    case CONDITIONS_C    : { checkCondition = c;    break; }
    case CONDITIONS_NC   : { checkCondition = !c;   break; }
    case CONDITIONS_Z    : { checkCondition = z;    break; }
    case CONDITIONS_NZ   : { checkCondition = !z;   break; }
  }

  if (checkCondition)
  {
    CTX->registerFile.programCounter = CTX->readData;
    cycles(1);
  }
}


// - - - API to get processors - - -
static Processor processors[] = 
  {
    [INSTRUCTION_NONE] = procNone,
    [INSTRUCTION_NOP]  = procNone,
    [INSTRUCTION_LOAD] = procLoad,
    [INSTRUCTION_JUMP] = procJump,
    [INSTRUCTION_DI]   = procDI,
    [INSTRUCTION_XOR]  = procXOR,
  };

Processor getInstrProcessor(InstructionType TYPE)
{ return processors[TYPE]; }
