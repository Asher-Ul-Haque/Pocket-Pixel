/**
 * @file serial.c
 * @brief Serial Data Transfer (Link Cable) implementation
 * * Uses an asynchronous delegate pattern. Instead of simulating an 8192Hz 
 * shift register cycle-by-cycle, it traps the hardware trigger and hands 
 * the payload off to the JavaScript platform layer, waiting for a callback.
*/

#include <bus.h>
#include <serial.h>
#include <cpu/interrupts.h>
#include <platform.h>

static SerialContext ctx;

SerialContext* serialGetContext(void) 
{ return &ctx; }

void serialInit(void) 
{
  ctx.sb              = SB_INIT_VALUE;
  ctx.sc              = SC_INIT_VALUE;
  ctx.isTransferring  = false;
}

u8 serialRead(u16 ADDRESS) 
{
  switch (ADDRESS) 
  {
    case ADDR_SB: return ctx.sb;
    case ADDR_SC: return ctx.sc | SC_MASK; 
    default:
      FORGE_LOG_ERROR("Attempted to read from invalid serial address: 0x%04X", ADDRESS);
      return OPEN_BUS_VALUE;
  }
}

void serialWrite(u16 ADDRESS, u8 VALUE) 
{
  switch (ADDRESS) 
  {
    case ADDR_SB: 
      ctx.sb = VALUE; 
      break;

    case ADDR_SC: 
      ctx.sc = VALUE;

      // - - - Check if Bit 7 (Transfer Enable) is flipped to 1
      if ((VALUE & SC_TRANSFER_ENABLE_BIT) != 0) 
      {
        // - - - Prevent duplicate overlapping requests if one is already flying over the network
        if (!ctx.isTransferring) 
        {
          ctx.isTransferring = true;
          
          bool isMaster = (VALUE & SC_CLOCK_SELECT_BIT) != 0;
          PlatformContext* platform = platformGetContext();
          
          // - - - Route to the Platform layer if it's hooked up
          if (platform && platform->serial.transferRequest) 
          {
            platform->serial.transferRequest(ctx.sb, isMaster);
          } 
          // - - - Fallback: If no network/platform is attached, instantly fake a disconnected cable (0xFF) so the game ROM doesn't freeze waiting.
          else 
          {
            coreCompleteSerialTransfer(OPEN_BUS_VALUE);
          }
        }
      }
      break;
        
    default:
      FORGE_LOG_ERROR("Attempted to write to invalid serial address: 0x%04X", ADDRESS);
      break;
  }
}

void coreCompleteSerialTransfer(u8 INCOMING_BYTE) 
{
  // - - - 0. Ignore stray or delayed packets if we aren't waiting
  if (!ctx.isTransferring) return;

  // - - - 1. Swap the internal buffer with the network response
  ctx.sb = INCOMING_BYTE;
  
  // - - - 2. Clear Bit 7 to signal to the game ROM's while-loop that the hardware is done
  ctx.sc &= ~SC_TRANSFER_ENABLE_BIT;
  
  // - - - 3. Unlock the subsystem for the next transfer
  ctx.isTransferring = false;
  
  // - - - 4. Fire the hardware interrupt to wake up the CPU
  cpuRequestInterrupt(CPU_INT_SERIAL);
}
