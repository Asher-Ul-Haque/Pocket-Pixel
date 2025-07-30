#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"

static SerialContext ctx;

SerialContext* serialGetContext()
{ return &ctx; }

void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB                      = 0x00;
  ctx.SC                      = 0x00;
  ctx.isTransfering           = false;
  ctx.newNetworkByteAvaiable  = false;
  ctx.recievedNetworkByte     = 0x00;
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    // - - - Serial Data register
    case 0xFF01 : 
      if (ctx.newNetworkByteAvaiable)
      {
        ctx.newNetworkByteAvaiable = false;
        return ctx.recievedNetworkByte;
      }
    return ctx.SB;

    // - - - Serial Control register 
    case 0xFF02 : return ctx.SC | 0x7C;

    default :
      FORGE_LOG_WARNING("Serial : Attempt to read from unkown serial address 0x%04X");
      return 0xFF;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    // - - - Serial Data register 
    case 0xFF01 : 
      ctx.SB = VALUE;
      break;

    // - - - Serial Control register 
    case 0xFF02 : 
      ctx.SC = (VALUE & 0x83);
      
      // - - - transfer bit is set 
      if (BIT(ctx.SC, 7))
      {
        // - - - internal clock 
        if (BIT(ctx.SC, 0))        
        {
          sendSerialByte(ctx.SB);
          ctx.SC            &= ~0x80;
          ctx.isTransfering =   false;
          cpuRequestInterrupt(IT_SERIAL);
        }
        else ctx.isTransfering = true;
      }
      break;

    default : 
      FORGE_LOG_WARNING("Serial : attempt to write to unkown serial address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE)
{
  if (ctx.isTransfering)
  {
    ctx.recievedNetworkByte     = BYTE;
    ctx.newNetworkByteAvaiable  = true;
    ctx.isTransfering           = false;

    ctx.SC &= ~0x80;
    cpuRequestInterrupt(IT_SERIAL);
  }
  else FORGE_LOG_WARNING("Serial : Recieved netowrk byte 0x%02X but not in slave transfer mode, ignoring", BYTE);
}
