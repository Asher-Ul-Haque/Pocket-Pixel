#include "../include/cpu.h"
#include "../include/bus.h"
#include "../include/emu.h"

extern CPUcontext ctx;

void fetchData() 
{
  ctx.memDest   = 0;
  ctx.destIsMem = false;
    
  if (ctx.currentInstruction == NULL) return;

  switch(ctx.currentInstruction->mode) 
  {
    case AM_IMP: return;

    case AM_R:
      ctx.readData = cpuReadRegister(ctx.currentInstruction->reg1);
      return;

    case AM_R_R:
      ctx.readData = cpuReadRegister(ctx.currentInstruction->reg2);
      return;

    case AM_R_D8:
      ctx.readData = busRead(ctx.regs.programCounter);
      emuCycles(1);
      ctx.regs.programCounter++;
      return;

    case AM_R_D16:
    case AM_D16: 
      {
        u16 lo = busRead(ctx.regs.programCounter);
        emuCycles(1);

        u16 hi = busRead(ctx.regs.programCounter + 1);
        emuCycles(1);

        ctx.readData             = lo | (hi << 8);
        ctx.regs.programCounter += 2;

        return;
      }

    case AM_MR_R:
      ctx.readData  = cpuReadRegister(ctx.currentInstruction->reg2);
      ctx.memDest   = cpuReadRegister(ctx.currentInstruction->reg1);
      ctx.destIsMem = true;

      if (ctx.currentInstruction->reg1 == RT_C) ctx.memDest |= 0xFF00;

      return;

    case AM_R_MR: 
      {
        u16 addr = cpuReadRegister(ctx.currentInstruction->reg2);

        if (ctx.currentInstruction->reg2 == RT_C)     addr |= 0xFF00;

        ctx.readData = busRead(addr);
        emuCycles(1);
      } return;

    case AM_R_HLI:
      ctx.readData = busRead(cpuReadRegister(ctx.currentInstruction->reg2));
      emuCycles(1);
      cpuSetRegister(RT_HL, cpuReadRegister(RT_HL) + 1);
      return;

    case AM_R_HLD:
      ctx.readData = busRead(cpuReadRegister(ctx.currentInstruction->reg2));
      emuCycles(1);
      cpuSetRegister(RT_HL, cpuReadRegister(RT_HL) - 1);
      return;

    case AM_HLI_R:
      ctx.readData = cpuReadRegister(ctx.currentInstruction->reg2);
      ctx.memDest = cpuReadRegister(ctx.currentInstruction->reg1);
      ctx.destIsMem = true;
      cpuSetRegister(RT_HL, cpuReadRegister(RT_HL) + 1);
      return;

    case AM_HLD_R:
      ctx.readData = cpuReadRegister(ctx.currentInstruction->reg2);
      ctx.memDest = cpuReadRegister(ctx.currentInstruction->reg1);
      ctx.destIsMem = true;
      cpuSetRegister(RT_HL, cpuReadRegister(RT_HL) - 1);
      return;

    case AM_R_A8:
      ctx.readData = busRead(ctx.regs.programCounter);
      emuCycles(1);
      ctx.regs.programCounter++;
      return;

    case AM_A8_R:
      ctx.memDest = busRead(ctx.regs.programCounter) | 0xFF00;
      ctx.destIsMem = true;
      emuCycles(1);
        ctx.regs.programCounter++;
        return;

    case AM_HL_SPR:
      ctx.readData = busRead(ctx.regs.programCounter);
      emuCycles(1);
      ctx.regs.programCounter++;
      return;

    case AM_D8:
      ctx.readData = busRead(ctx.regs.programCounter);
      emuCycles(1);
      ctx.regs.programCounter++;
      return;

    case AM_A16_R:
    case AM_D16_R: 
      {
        u16 lo = busRead(ctx.regs.programCounter);
        emuCycles(1);

        u16 hi = busRead(ctx.regs.programCounter + 1);
        emuCycles(1);

        ctx.memDest   = lo | (hi << 8);
        ctx.destIsMem = true;

        ctx.regs.programCounter += 2;
        ctx.readData             = cpuReadRegister(ctx.currentInstruction->reg2);

      } return;

    case AM_MR_D8:
      ctx.readData = busRead(ctx.regs.programCounter);
      emuCycles(1);
      ctx.regs.programCounter++;
      ctx.memDest   = cpuReadRegister(ctx.currentInstruction->reg1);
      ctx.destIsMem = true;
      return;

    case AM_MR:
      ctx.memDest   = cpuReadRegister(ctx.currentInstruction->reg1);
      ctx.destIsMem = true;
      ctx.readData  = busRead(cpuReadRegister(ctx.currentInstruction->reg1));
      emuCycles(1);
      return;

    case AM_R_A16: 
      {
        u16 lo = busRead(ctx.regs.programCounter);
        emuCycles(1);

        u16 hi = busRead(ctx.regs.programCounter + 1);
        emuCycles(1);

        u16 addr = lo | (hi << 8);

        ctx.regs.programCounter += 2;
        ctx.readData             = busRead(addr);
        emuCycles(1);

        return;
      }

    default:
      FORGE_LOG_FATAL("Unknown Addressing Mode! %d (%02X)\n", ctx.currentInstruction->mode, ctx.currentOpcode);
      FORGE_ASSERT(false);
      return;
    }
}
