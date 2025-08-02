#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"
#include <cstring>

static SerialContext ctx;
static u8            queuedByte     = 0xFF;
static bool          hasQueuedByte  = false;

void serialCompleteTransfer(u8 RECEIVED_BYTE);


SerialContext* serialGetContext() { return &ctx; }

void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB                      = 0x00;
  ctx.SC                      = 0x00;
  ctx.isTransfering           = false;
  ctx.newNetworkByteAvaiable  = false;
  ctx.recievedNetworkByte     = 0x00;

  queuedByte    = 0xFF;
  hasQueuedByte = false;

  FORGE_LOG_INFO("Serial: Initialized.");
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    // - - - SB register
    case 0xFF01:
      return ctx.SB;

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
    {
      u8 oldSC  = ctx.SC;
      ctx.SC    = VALUE & 0x83;

      bool isInternalClock = BIT(ctx.SC, 0);
      bool transferStarted = BIT(ctx.SC, 7);

      FORGE_LOG_INFO("Serial: SC write - Old: 0x%02X, New: 0x%02X, Internal clock: %d",
                     oldSC, ctx.SC, isInternalClock);

      if (transferStarted)
      {
        ctx.isTransfering = true;

        // - - - Master mode (SC = 0x81)
        if (isInternalClock)
        {
          FORGE_LOG_INFO("Serial: MASTER - Initiating transfer with byte 0x%02X", ctx.SB);
          sendSerialByte(ctx.SB);

          // - - - If we already have a queued response from slave, complete immediately
          if (hasQueuedByte)
          {
            FORGE_LOG_INFO("Serial: MASTER - Using queued slave response: 0x%02X", queuedByte);
            serialCompleteTransfer(queuedByte);
            hasQueuedByte = false;
          }
        }

        // - - - Slave mode (SC = 0x80)
        else
        {
          FORGE_LOG_INFO("Serial: SLAVE - Ready to receive, will respond with byte 0x%02X", ctx.SB);

          // - - - If master already sent us a byte, respond immediately
          if (hasQueuedByte)
          {
            FORGE_LOG_INFO("Serial: SLAVE - Responding to master byte 0x%02X with 0x%02X",
                           queuedByte, ctx.SB);
            sendSerialByte(ctx.SB);
            serialCompleteTransfer(queuedByte);
            hasQueuedByte = false;
          }
          // - - - Otherwise wait for master to send first
        }
      }
    }
      break;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to write to unknown address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE)
{
  FORGE_LOG_INFO("Serial: Received byte from network: 0x%02X", BYTE);

  if (ctx.isTransfering)
  {
    bool weAreUsingInternalClock = BIT(ctx.SC, 0);

    // - - - We are master
    if (weAreUsingInternalClock)
    {
      FORGE_LOG_INFO("Serial: MASTER - Received slave response: 0x%02X", BYTE);
      serialCompleteTransfer(BYTE);
    }
    // - - - We are slave
    else
    {
      FORGE_LOG_INFO("Serial: SLAVE - Received master request: 0x%02X, responding with: 0x%02X",
                     BYTE, ctx.SB);
      sendSerialByte(ctx.SB);
      serialCompleteTransfer(BYTE);
    }
  }
  else
  {
    // - - - Queue for later
    queuedByte    = BYTE;
    hasQueuedByte = true;
    FORGE_LOG_INFO("Serial: Queued byte 0x%02X for later", BYTE);
  }
}

void serialCompleteTransfer(u8 RECEIVED_BYTE)
{
  FORGE_LOG_INFO("Serial: Transfer complete - SB = 0x%02X", RECEIVED_BYTE);

  // - - - Clear transfer bit
  ctx.SB             = RECEIVED_BYTE;
  ctx.SC            &= ~0x80;
  ctx.isTransfering  = false;

  cpuRequestInterrupt(IT_SERIAL);
}