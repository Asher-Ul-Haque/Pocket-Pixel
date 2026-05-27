#include "utils/logger.h"
#include <debug.h>
#include <stdio.h>

static DebugContext ctx;
DebugContext* debugGetContext(void) { return &ctx; } 

static void serialAppend(char CHAR)
{
  if (ctx.index < SERIAL_BUFFER_SIZE - 1) 
  {
    ctx.string[ctx.index++] = CHAR;
    ctx.string[ctx.index] = '\0';
  }
}

static void serialFlushToTerminal(void)
{
  printf("\033[2J\033[H");
  printf("%s", ctx.string);
  fflush(stdout);
}

void serialInit(void)
{
  ctx.SB = 0;
  ctx.SC = 0;
  ctx.index = 0;
  memset(ctx.string, 0, sizeof(ctx.string));
}

u8 serialRead(u16 ADDR)
{
  if (ADDR == 0xFF01) return ctx.SB;
  if (ADDR == 0xFF02) return ctx.SC;
  return 0xFF;
}

void serialWrite(u16 ADDR, u8 VALUE)
{
  if (ADDR == 0xFF01) 
  {
     ctx.SB = VALUE;
     return;
  }

  if (ADDR == 0xFF02) 
  {
    ctx.SC = VALUE;

    // Transfer trigger (blargg convention)
    if (VALUE == 0x81) 
    {
      serialAppend((char)ctx.SB);
      serialFlushToTerminal();
    }
    return;
  }
}
