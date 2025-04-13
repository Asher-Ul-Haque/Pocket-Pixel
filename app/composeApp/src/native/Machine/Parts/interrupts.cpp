#include "interrupts.h"
#include "stack.h"
#include "cpu.h"

void intHandle(CPUContext* ctx, u16 addr)
{
    stackPush16(ctx->registerFile.programCounter);
    ctx->registerFile.programCounter = addr;
}

bool intCheck(CPUContext* ctx, u16 addr, interrupt_type it)
{
    if(ctx->intFlags & it && ctx->ieRegister & it)
    {
        intHandle(ctx,addr);
        ctx->intFlags &= ~1;
        ctx->halted = false;
        ctx->instructionMasterEnabled = false;

        return true;
    }
    return false;
}

void cpuHandleInterrupts(CPUContext* ctx)
{
    if (intCheck(ctx, 0x40, IT_VBLANK)) {}
    else if (intCheck(ctx, 0x48, IT_LCD_STAT)) {}
    else if (intCheck(ctx, 0x50, IT_TIMER)){}
    else if (intCheck(ctx, 0x58, IT_SERIAL)){}
    else if (intCheck(ctx, 0x60, IT_JOYPAD)){}
}
