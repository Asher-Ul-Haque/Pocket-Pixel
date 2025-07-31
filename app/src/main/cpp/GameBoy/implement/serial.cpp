#include "../include/serial.h"
#include "../include/bus.h"
#include "../include/interrupt.h"
#include "../include/common.h"
#include "../../GameBoyCore.h"

static SerialContext ctx;

// Network transfer state
static bool hasQueuedByte = false;
static u8 queuedNetworkByte = 0x00;
// Remove static master/slave assignment - let the games decide!

SerialContext* serialGetContext()
{
  return &ctx;
}

void serialCompleteTransfer(u8 receivedByte);
void serialInit()
{
  memset(&ctx, 0, sizeof(SerialContext));
  ctx.SB                      = 0x00;
  ctx.SC                      = 0x00;
  ctx.isTransfering           = false;
  ctx.newNetworkByteAvaiable  = false;
  ctx.recievedNetworkByte     = 0x00;

  // Reset network state
  hasQueuedByte = false;
  queuedNetworkByte = 0x00;
}

u8 serialRead(u16 ADDRESS)
{
  switch (ADDRESS)
  {
    case 0xFF01: // SB register
      return ctx.SB;

    case 0xFF02: // SC register  
      return ctx.SC | 0x7C; // Bits 6-2 always read as 1

    default:
      FORGE_LOG_WARNING("Serial: Attempt to read from unknown serial address 0x%04X", ADDRESS);
      return 0xFF;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE)
{
  switch (ADDRESS)
  {
    case 0xFF01: // SB register
      ctx.SB = VALUE;
      break;

    case 0xFF02: // SC register
      {
        bool oldTransferBit = BIT(ctx.SC, 7);
        ctx.SC = (VALUE & 0x83); // Only bits 7,1,0 are writable
        bool newTransferBit = BIT(ctx.SC, 7);
        bool isInternalClock = BIT(ctx.SC, 0);

        // Transfer started (0->1 transition on bit 7)
        if (!oldTransferBit && newTransferBit)
        {
          if (isInternalClock) // Master mode (SC = $81)
          {
            FORGE_LOG_TRACE("Serial: Acting as master, starting transfer with byte 0x%02X", ctx.SB);
            ctx.isTransfering = true;
            
            // Master sends its byte immediately
            sendSerialByte(ctx.SB);
            
            // If other device already prepared a response, complete immediately
            if (hasQueuedByte)
            {
              serialCompleteTransfer(queuedNetworkByte);
              hasQueuedByte = false;
            }
            // Otherwise wait for other device's response via serialReceiveNetworkByte()
          }
          else // Slave mode (SC = $80) - external clock
          {
            FORGE_LOG_TRACE("Serial: Acting as slave, ready for transfer, will send byte 0x%02X", ctx.SB);
            ctx.isTransfering = true;
            
            // If other device already sent us a byte, respond immediately
            if (hasQueuedByte)
            {
              FORGE_LOG_TRACE("Serial: Slave responding to queued master byte 0x%02X with 0x%02X", 
                             queuedNetworkByte, ctx.SB);
              sendSerialByte(ctx.SB);
              serialCompleteTransfer(queuedNetworkByte);
              hasQueuedByte = false;
            }
            // Otherwise wait for other device to send a byte
          }
        }
      }
      break;

    default:
      FORGE_LOG_WARNING("Serial: Attempt to write to unknown serial address 0x%04X with value 0x%02X", ADDRESS, VALUE);
      break;
  }
}

void serialReceiveNetworkByte(u8 BYTE)
{
  FORGE_LOG_TRACE("Serial: Received byte from network: 0x%02X", BYTE);

  if (ctx.isTransfering)
  {
    // Determine our role based on our current SC register state
    bool weAreUsingInternalClock = BIT(ctx.SC, 0);
    
    if (weAreUsingInternalClock)
    {
      // We are master, this is the slave's response
      FORGE_LOG_TRACE("Serial: Master completing transfer with slave response: 0x%02X", BYTE);
      serialCompleteTransfer(BYTE);
    }
    else
    {
      // We are slave, this is master's byte - we should respond
      FORGE_LOG_TRACE("Serial: Slave received master byte: 0x%02X, responding with: 0x%02X", 
                     BYTE, ctx.SB);
      sendSerialByte(ctx.SB);
      serialCompleteTransfer(BYTE);
    }
  }
  else
  {
    // Queue the byte for when transfer starts
    queuedNetworkByte = BYTE;
    hasQueuedByte = true;
    FORGE_LOG_TRACE("Serial: Queued byte 0x%02X for later transfer", BYTE);
  }
}

void serialCompleteTransfer(u8 receivedByte)
{
  FORGE_LOG_TRACE("Serial: Completing transfer, SB = 0x%02X", receivedByte);
  
  // According to Pan Docs: SB contains the received byte after transfer
  ctx.SB = receivedByte;
  
  // Clear transfer bit (bit 7) - this indicates transfer is complete
  ctx.SC &= ~0x80;
  
  // Transfer is no longer active
  ctx.isTransfering = false;
  
  // Request serial interrupt
  cpuRequestInterrupt(IT_SERIAL);
}

// Helper function to check if disconnected (for timeout handling)
bool serialIsDisconnected()
{
  // On disconnected cable, master receives 0xFF
  bool weAreUsingInternalClock = BIT(ctx.SC, 0);
  return (weAreUsingInternalClock && ctx.isTransfering && ctx.SB == 0xFF);
}
