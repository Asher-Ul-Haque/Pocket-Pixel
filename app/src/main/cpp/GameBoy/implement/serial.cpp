#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"

#include <cstring> // For memset

static SerialContext ctx;

SerialContext* serialGetContext() { return &ctx; }

void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB                      = 0x00;
  ctx.SC                      = 0x00;
  ctx.isTransfering           = false;
  ctx.newNetworkByteAvaiable  = false;
  ctx.recievedNetworkByte     = 0x00;
  FORGE_LOG_INFO("Serial: Initialized.");
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    // - - - SB register
    case 0xFF01:
    {
      if (ctx.newNetworkByteAvaiable)
      {
        ctx.newNetworkByteAvaiable = false;
        return ctx.recievedNetworkByte;
      }
      return ctx.SB;
    }

    // - - - SC register
    case 0xFF02:
      FORGE_LOG_TRACE("Serial: Reading SC (0xFF02) value: 0x%02X.", ctx.SC | 0x7C);
      return ctx.SC | 0x7C;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to read from unknown address 0x%04X", ADDRESS);
      return 0xFF;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    // - - - SB register
    case 0xFF01:
      ctx.SB = VALUE;
      break;

    // - - - SC Register
    case 0xFF02:
      ctx.SC = VALUE & 0x83;

      // - - - Check if transfer start bit (Bit 7) is set
      if (BIT(ctx.SC, 7))
      {
        ctx.isTransfering           = true;
        ctx.newNetworkByteAvaiable  = false;
        sendSerialByte(ctx.SB);
        FORGE_LOG_INFO("Serial: Transfer initiated. Sending byte 0x%02X. Waiting for response...", ctx.SB);
      }
      break;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to write to unknown address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE)
{
  FORGE_LOG_INFO("Serial: Received BYTE from network: 0x%02X.", BYTE);

  // - - - This byte is the *response* from the other Game Boy.
  // - - - It should only be processed if a transfer was initiated by the local GB.
  if (ctx.isTransfering)
  {
    ctx.recievedNetworkByte     = BYTE;
    ctx.newNetworkByteAvaiable  = true;
    ctx.isTransfering           = false;

    // - - - Clear the transfer start bit (Bit 7) in SC. Transfer complete from this perspective
    ctx.SC &= ~0x80;
    cpuRequestInterrupt(IT_SERIAL);
  }
  else
  { FORGE_LOG_WARNING("Serial: Received byte 0x%02X but not in transfer mode (isTransfering: %d) – ignoring.", BYTE, ctx.isTransfering); }
}
