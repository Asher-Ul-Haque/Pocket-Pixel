#include "../include/debugger.h"
#include "../include/bus.h"

static char debugMessage[1024]  = {0};
static u16  messageSize         = 0;

void debuggerUpdate()
{
  if (busRead(0xFF02) == 0x81)
  {
    char c = busRead(0xFF01);
    FORGE_LOG_TRACE("Read : %c", c);

    debugMessage[messageSize++] = c;

    busWrite(0xFF02, 0);
  }
}

void debuggerPrint()
{
  if (debugMessage[0]) FORGE_LOG_DEBUG("MSG : %s", debugMessage);
}
