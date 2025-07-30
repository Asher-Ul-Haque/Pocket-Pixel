#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"

static SerialContext ctx;

SerialContext* serialGetContext() {
  return &ctx;
}

void serialInit() {
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB = 0x00;
  ctx.SC = 0x00;
  ctx.isTransfering = false;
  ctx.newNetworkByteAvaiable = false;
  ctx.recievedNetworkByte = 0x00;
}

u8 serialRead(u16 ADDRESS) {
  switch (ADDRESS) {
    case 0xFF01: {
      if (ctx.newNetworkByteAvaiable) {
        ctx.newNetworkByteAvaiable = false;
        FORGE_LOG_TRACE("Serial: Reading received network byte 0x%02X", ctx.recievedNetworkByte);
        return ctx.recievedNetworkByte;
      }
      return ctx.SB;
    }

    case 0xFF02:
      return ctx.SC | 0x7C;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to read from unknown address 0x%04X", ADDRESS);
      return 0xFF;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE) {
  switch (ADDRESS) {
    case 0xFF01:
      ctx.SB = VALUE;
      FORGE_LOG_TRACE("Serial: Wrote to SB (0xFF01) value: 0x%02X", VALUE);
      break;

    case 0xFF02:
      ctx.SC = VALUE & 0x83;
      FORGE_LOG_TRACE("Serial: Wrote to SC (0xFF02) value: 0x%02X", VALUE);

      // Start transfer if bit 7 set
      if (BIT(ctx.SC, 7)) {
        if (BIT(ctx.SC, 0)) {
          // Internal clock – instant transfer (for testing)
          FORGE_LOG_TRACE("Serial: Internal clock transfer mode, sending byte immediately.");
          sendSerialByte(ctx.SB);

          // End transfer
          ctx.SC &= ~0x80;
          ctx.isTransfering = false;
          cpuRequestInterrupt(IT_SERIAL);
        } else {
          // External clock – wait for response
          FORGE_LOG_TRACE("Serial: External clock, waiting for network response. Byte to send: 0x%02X", ctx.SB);
          ctx.isTransfering = true;
          sendSerialByte(ctx.SB);  // Send to websocket or browser
        }
      }
      break;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to write to unknown address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE) {
  FORGE_LOG_TRACE("Serial: Received BYTE from network: 0x%02X", BYTE);

  if (ctx.isTransfering) {
    ctx.recievedNetworkByte = BYTE;
    ctx.newNetworkByteAvaiable = true;
    ctx.isTransfering = false;

    ctx.SC &= ~0x80;  // Clear transfer bit
    cpuRequestInterrupt(IT_SERIAL);

    FORGE_LOG_TRACE("Serial: Transfer complete, interrupt requested.");
  } else {
    FORGE_LOG_WARNING("Serial: Received byte 0x%02X but not in transfer mode – ignoring.", BYTE);
  }
}
