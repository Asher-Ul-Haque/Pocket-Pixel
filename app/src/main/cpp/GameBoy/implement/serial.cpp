#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"

static SerialContext ctx;

SerialContext* serialGetContext() { return &ctx; }

void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB                     = 0x00;
  ctx.SC                     = 0x00;
  ctx.isTransfering          = false;
  ctx.newNetworkByteAvaiable = false;
  ctx.recievedNetworkByte    = 0x00;

  FORGE_LOG_INFO("Serial: Initialized.");
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    // - - - SB
    case 0xFF01:
      if (ctx.newNetworkByteAvaiable)
      {
        ctx.newNetworkByteAvaiable = false;
        FORGE_LOG_TRACE("Serial: Reading received byte 0x%02X from SB (0xFF01).", ctx.recievedNetworkByte);
        return ctx.recievedNetworkByte;
      }
      FORGE_LOG_TRACE("Serial: Reading SB (0xFF01): 0x%02X (no new byte)", ctx.SB);
      return ctx.SB;

    // - - - SC
    case 0xFF02:
      return ctx.SC | 0x7C;

    default:
      FORGE_LOG_WARNING("Serial: Read from unknown address 0x%04X", ADDRESS);
      return 0xFF;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    // - - - SB
    case 0xFF01:
      ctx.SB = VALUE;
      FORGE_LOG_TRACE("Serial: Wrote 0x%02X to SB (0xFF01).", VALUE);
      break;

    // - - - SC
    case 0xFF02:
      ctx.SC = VALUE & 0x83;
      FORGE_LOG_TRACE("Serial: Wrote 0x%02X to SC (0xFF02).", ctx.SC);

      if (BIT(ctx.SC, 7))
      {
        if (!ctx.isTransfering)
        {
          ctx.isTransfering           = true;
          ctx.newNetworkByteAvaiable  = false;
          FORGE_LOG_INFO("Serial: Transfer initiated. Sending SB=0x%02X, SC=0x%02X", ctx.SB, ctx.SC);
          sendSerialByte(ctx.SB, ctx.SC);
        }
        else
        {
          FORGE_LOG_WARNING("Serial: Attempted to start new transfer while already in progress.");
        }
      }
      break;

    default:
      FORGE_LOG_WARNING("Serial: Write to unknown address 0x%04X (value=0x%02X)", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE)
{
  if (!ctx.isTransfering)
  {
    FORGE_LOG_WARNING("Serial: Received byte 0x%02X but not in transfer mode. Ignoring.", BYTE);
    return;
  }

  // - - - Store received byte into SB
  ctx.recievedNetworkByte     = BYTE;
  ctx.newNetworkByteAvaiable  = true;
  ctx.isTransfering           = false;

  // - - - Clear bit 7 (transfer start bit) in SC
  ctx.SC &= ~0x80;

  // - - - Request serial interrupt
  cpuRequestInterrupt(IT_SERIAL);

  FORGE_LOG_INFO("Serial: Transfer complete. Received=0x%02X. Interrupt requested.", BYTE);
}
