#include "../include/debugger.h"
#include "../include/bus.h"

static char debugMesage[1024] = {0};
static u64  messageSize       = 0;

void debuggerUpdate() 
{
  if (busRead(0xFF02) == 0x81) 
  {
    char c                      = busRead(0xFF01);
    debugMesage[messageSize++]  = c;
    busWrite(0xFF02, 0);
  }
}

void debuggerPrint() 
{
  if (debugMesage[0]) FORGE_LOG_DEBUG("%s", debugMesage);
}
