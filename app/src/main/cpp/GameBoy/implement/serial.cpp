#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"


static SerialContext ctx;


void serialCompleteTransfer(u8 RECEIVED_BYTE)
{
  ctx.recievedNetworkByte     = RECEIVED_BYTE;
  ctx.newNetworkByteAvaiable  = true;           // - - - Mark new byte available for CPU read
  ctx.isTransfering           = false;          // - - - Transfer is now complete

  // - - - Clear the transfer start bit (Bit 7) in SC. This is crucial.
  ctx.SC &= ~0x80;

  // - - - Request a Serial Interrupt (Interrupt Flag register bit 3).
  cpuRequestInterrupt(IT_SERIAL);

  FORGE_LOG_INFO("Serial: Transfer completed. SB updated to 0x%02X. Interrupt requested.", RECEIVED_BYTE);
}

SerialContext* serialGetContext() {return &ctx;}

void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB = 0x00;
  ctx.SC = 0x00;
  ctx.isTransfering = false;
  ctx.newNetworkByteAvaiable = false;
  ctx.recievedNetworkByte = 0x00;
  FORGE_LOG_INFO("Serial: Initialized.");
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    // - - - SB
    case 0xFF01:
    {
      if (ctx.newNetworkByteAvaiable)
      {
        ctx.newNetworkByteAvaiable = false;
        FORGE_LOG_TRACE("Serial: Reading received network byte 0x%02X from SB (0xFF01).", ctx.recievedNetworkByte);
        return ctx.recievedNetworkByte;
      }
      FORGE_LOG_TRACE("Serial: Reading current SB (0xFF01) value: 0x%02X (no new network byte).", ctx.SB);
      return ctx.SB; // - - - If no new byte, return the last byte written/transferred
    }

    // - - - SC Register
    case 0xFF02:
      // - - - Bits 1-6 are always 1 when read on DMG, except for CGB speed bit (bit 1)
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
    // - - - SB Register
    case 0xFF01:
      ctx.SB = VALUE;
      FORGE_LOG_TRACE("Serial: Wrote to SB (0xFF01) value: 0x%02X.", VALUE);
      break;

    // - - - SC Register
    case 0xFF02:
      ctx.SC = VALUE & 0x83;
      FORGE_LOG_TRACE("Serial: Wrote to SC (0xFF02) value: 0x%02X.", VALUE);

      // - - - Check if transfer start bit (Bit 7) is set
      if (BIT(ctx.SC, 7))
      {
        // - - - A transfer is initiated by the Game Boy.
        ctx.isTransfering          = true;
        ctx.newNetworkByteAvaiable = false;
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

  // - - - This byte is the response from the other Game Boy.
  if (ctx.isTransfering)  serialCompleteTransfer(BYTE);
  else                    FORGE_LOG_WARNING("Serial: Received byte 0x%02X but not in transfer mode (isTransfering: %d) – ignoring.", BYTE, ctx.isTransfering);

}
