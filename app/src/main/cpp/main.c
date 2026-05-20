#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <utils/logger.h>
#include <debug.h>
#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <utils/logger.h>
#include <debug.h>
#include <SDL3/SDL.h>

#define DOTS_PER_FRAME  70224
#define DOTS_PER_MCYCLE 4

static char savePath[1024];

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE) 
{
  if (savePath[0] == '\0') 
  {
    FORGE_LOG_ERROR("%s", "Save path is not set. Cannot save RAM.");
    return false;
  }
  FILE* f = fopen(savePath, "wb");
  if (!f) 
  {
    FORGE_LOG_ERROR("Failed to open save file for writing: %s", savePath);
    return false;
  }
  u64 written = fwrite(RAM_DATA, 1, RAM_SIZE, f);
  fclose(f);
  return written == RAM_SIZE;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE) 
{
  if (savePath[0] == '\0') 
  {
    FORGE_LOG_ERROR("%s", "Save path is not set. Cannot load RAM.");
    return false;
  }
  FILE* f = fopen(savePath, "rb");
  if (!f) 
  {
    FORGE_LOG_ERROR("Failed to open save file for reading: %s. Starting with empty RAM.", savePath);
    return false;
  }
  u64 read = fread(RAM_DATA, 1, RAM_SIZE, f);
  fclose(f);
  return read == RAM_SIZE;
}

u32 fileGetExpectedSaveSize(void) { return 0; }

i32 main(int ARGUMENT_COUNT, char* ARGUMENT_VECTOR[]) 
{
  if (ARGUMENT_COUNT < 2) 
  {
    FORGE_LOG_FATAL("Usage: %s <rom_file>", ARGUMENT_VECTOR[0]);
    return 1;
  }

  const char* romPath = ARGUMENT_VECTOR[1];
  FORGE_LOG_DEBUG("Attempting to open ROM: %s", romPath);

  FILE* f = fopen(romPath, "rb");
  if (!f) 
  {
    FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  u64 romSize = ftell(f);
  fseek(f, 0, SEEK_SET);
  FORGE_LOG_DEBUG("ROM size calculated: %llu bytes", romSize);

  u8* romData = (u8*)malloc(romSize);
  if (!romData) 
  {
     FORGE_LOG_FATAL("%s", "Memory allocation failed for ROM data");
     fclose(f);
     return 1;
  }

  if (fread(romData, 1, romSize, f) != (u64)romSize) 
  {
    FORGE_LOG_ERROR("%s", "Failed to read ROM data");
    free(romData);
    fclose(f);
    return 1;
  }
  fclose(f);
  FORGE_LOG_DEBUG("%s", "ROM read successfully.");

  snprintf(savePath, sizeof(savePath), "%s.sav", romPath);

  CartridgeFileIO fileIO = 
  {
    .saveRamToFile       = fileSaveRam,
    .loadRamFromFile     = fileLoadRam,
    .getExpectedSaveSize = fileGetExpectedSaveSize
  };

  if (!cartridgeInit(&fileIO, romData, (u32)romSize)) 
  {
    FORGE_LOG_FATAL("%s", "Failed to initialize cartridge");
    free(romData);
    return 1;
  }

  platformInit();
  cpuInit();
  ppuInit();
  timerInit();
  serialInit();

  FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");
  bool running = true;
  PlatformContext* platform = platformGetContext();
  PpuContext* ppu = ppuGetContext();

  while (running) 
  {
    // Emulate exactly one hardware frame interval
    for (u32 frame_cycles = 0; frame_cycles < DOTS_PER_FRAME; ) 
    {
      cpuTick(); 
      timerStepMCycle();
      
      // Step the PPU by 1 M-Cycle worth of active clock dots
      ppuTick(DOTS_PER_MCYCLE); 
      frame_cycles += DOTS_PER_MCYCLE;
    }

    static u32 drawn_frames = 0;
    FORGE_LOG_DEBUG("Presenting Frame: %u", ++drawn_frames);

    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
      if (event.type == SDL_EVENT_QUIT) 
      {
        running = false;
      }
    }

    // Refresh display matrix panels dynamically using encapsulated pointers
    if (platform && ppu) {
        platform->rendering.drawTileView(ppu->vram[0], ppu->vram[1]);
        platform->rendering.drawMapView(ppu->vram[0], ppu->vram[1], 0);
        platform->rendering.present();
    }
  }

  platform->rendering.cleanup();
  free(romData);
  return 0;
}
