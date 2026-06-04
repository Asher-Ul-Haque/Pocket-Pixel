#include <state.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <apu/apu.h>
#include <cartridge/cartridge.h>
#include <joypad.h>
#include <timer.h>
#include <ram.h>
#include <stdlib.h>
#include <string.h>

u8* systemSaveStateToMemory(u32* OUT_SIZE)
{
  CartContext* cart = cartridgeGetContext();

  // - - - 1. Calculate exact buffer size needed
  u32 totalSize = sizeof(CpuContext) + 
                  sizeof(PpuContext) + 
                  sizeof(ApuContext) + 
                  sizeof(TimerContext) + 
                  sizeof(RamContext) + 
                  sizeof(JoypadContext) + 
                  sizeof(CartContext);
  
  if (cart->hasRam && cart->externalRamSize > 0 && cart->externalRamData) 
  {
    totalSize += cart->externalRamSize;
  }

  // - - - 2. Allocate the binary array
  u8* buffer = (u8*)malloc(totalSize);
  if (!buffer) 
  {
    *OUT_SIZE = 0;
    return NULL;
  }

  // - - - 3. Pack the data sequentially
  u8* ptr = buffer;

  memcpy(ptr, cpuGetContext(), sizeof(CpuContext));       ptr += sizeof(CpuContext);
  memcpy(ptr, ppuGetContext(), sizeof(PpuContext));       ptr += sizeof(PpuContext);
  memcpy(ptr, apuGetContext(), sizeof(ApuContext));       ptr += sizeof(ApuContext);
  memcpy(ptr, timerGetContext(), sizeof(TimerContext));   ptr += sizeof(TimerContext);
  memcpy(ptr, ramGetContext(), sizeof(RamContext));       ptr += sizeof(RamContext);
  memcpy(ptr, joypadGetContext(), sizeof(JoypadContext)); ptr += sizeof(JoypadContext);
  
  memcpy(ptr, cart, sizeof(CartContext));                 ptr += sizeof(CartContext);

  if (cart->hasRam && cart->externalRamSize > 0 && cart->externalRamData) 
  {
    memcpy(ptr, cart->externalRamData, cart->externalRamSize);
  }

  *OUT_SIZE = totalSize;
  return buffer;
}

bool systemLoadStateFromMemory(const u8* BUFFER, u32 SIZE)
{
  if (!BUFFER || SIZE == 0) return false;

  // - - - Minimum size is all contexts without external RAM
  u32 minSize = sizeof(CpuContext)    + 
                sizeof(PpuContext)    + 
                sizeof(ApuContext)    + 
                sizeof(TimerContext)  + 
                sizeof(RamContext)    + 
                sizeof(JoypadContext) + 
                sizeof(CartContext);
  
  if (SIZE < minSize) return false; // - - - Corrupted or invalid buffer

  CartContext* cart = cartridgeGetContext();
  ApuContext*  apu  = apuGetContext();

  // - - - Cache pointers and user-settings (volumes/speed)
  const u8*           origRom      = cart->romData;
  CartridgeMetadata*  origMeta     = cart->metadata;
  u8*                 origRam      = cart->externalRamData;
  CartridgeFileIO*    origIO       = cart->fileIO;
  u32                 origRamSize  = cart->externalRamSize;

  f32                 origSpeed    = apu->speedMultiplier;
  f32                 origVols[4];
  memcpy(origVols, apu->channelModifiers, sizeof(origVols));

  // - - - 1. Unpack the data sequentially
  const u8* ptr = BUFFER;

  memcpy(cpuGetContext(), ptr, sizeof(CpuContext));       ptr += sizeof(CpuContext);
  memcpy(ppuGetContext(), ptr, sizeof(PpuContext));       ptr += sizeof(PpuContext);
  memcpy(apuGetContext(), ptr, sizeof(ApuContext));       ptr += sizeof(ApuContext);
  memcpy(timerGetContext(), ptr, sizeof(TimerContext));   ptr += sizeof(TimerContext);
  memcpy(ramGetContext(), ptr, sizeof(RamContext));       ptr += sizeof(RamContext);
  memcpy(joypadGetContext(), ptr, sizeof(JoypadContext)); ptr += sizeof(JoypadContext);
  
  memcpy(cart, ptr, sizeof(CartContext));                 ptr += sizeof(CartContext);

  // - - - restore pointers and user-settings
  cart->romData         = origRom;
  cart->metadata        = origMeta;
  cart->externalRamData = origRam;
  cart->fileIO          = origIO;
  cart->externalRamSize = origRamSize;

  apu->speedMultiplier  = origSpeed;
  memcpy(apu->channelModifiers, origVols, sizeof(origVols));

  // - - - 2. Unpack External RAM if present in the save
  if (cart->hasRam && cart->externalRamSize > 0 && cart->externalRamData) 
  {
    u32 expectedRemaining = SIZE - minSize;
    if (expectedRemaining >= cart->externalRamSize) 
    {
       memcpy(cart->externalRamData, ptr, cart->externalRamSize);
    }
  }

  return true;
}
