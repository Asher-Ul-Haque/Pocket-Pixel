#include <io/cartridge.h>
#include <cpu/cpu.h>

static void setPair(RegType PAIR, u16 VALUE)
{
  RegisterFile* regs = &cpuGetContext()->registers;
  switch (PAIR)
  {
    case RT_AF: regs->a = (u8)(VALUE >> 8); regs->f = (u8)(VALUE & 0xF0u); break;
    case RT_BC: regs->b = (u8)(VALUE >> 8); regs->c = (u8)(VALUE & 0xFFu); break;
    case RT_DE: regs->d = (u8)(VALUE >> 8); regs->e = (u8)(VALUE & 0xFFu); break;
    case RT_HL: regs->h = (u8)(VALUE >> 8); regs->l = (u8)(VALUE & 0xFFu); break;
    default: 
      FORGE_LOG_ERROR("[CPU] : Invalid register pair %d", PAIR);
      break;
  }
}

void cpuInit(void)
{
  CartContext* cartCtx = cartridgeGetContext();
  CpuContext*  cpuCtx  = cpuGetContext();
  FORGE_ASSERT_DEBUG(cartCtx->initialized, "Cartridge must be initialized before calling cpuInitPostBootFromCartridge");

  memset(cpuCtx, 0, sizeof(CpuContext));

  // - - - always start at normal speed 
  cpuCtx->doubleSpeed = false;

  cpuCtx->halted                  = false;
  cpuCtx->stopped                 = false;
  cpuCtx->interruptMasterEnabled  = false;
  cpuCtx->enablingIme             = false;

  cpuCtx->registers.programCounter = START_VALUE_PROGRAM_COUNTER;
  cpuCtx->registers.stackPointer   = START_VALUE_STACK_POINTER;

  // - - - Initialize registers based on cartridge type
  if (cartCtx->mode == MODE_DMG_GAMEBOY)
  {
    setPair(RT_AF, START_VALUE_AF_DMG);
    setPair(RT_BC, START_VALUE_BC_DMG);
    setPair(RT_DE, START_VALUE_DE_DMG);
    setPair(RT_HL, START_VALUE_HL_DMG);
  }
  else
  {
    setPair(RT_AF, START_VALUE_AF_CGB);
    setPair(RT_BC, START_VALUE_BC_CGB);
    setPair(RT_DE, START_VALUE_DE_CGB);
    setPair(RT_HL, START_VALUE_HL_CGB);
  }

  cpuCtx->registers.f &= 0xF0u; // - - - lower 4 bits of F are always 0
}
