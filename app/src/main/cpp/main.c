#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <ppu/dma.h>
#include <cartridge/cartridge.h>
#include <utils/logger.h>
#include <SDL3/SDL.h>


// - - - Static Platform Helpers - - -

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

  // - - - 1. Read ROM file into memory
  FILE* f = fopen(romPath, "rb");
  if (!f) 
  {
    FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  u64 romSize = ftell(f);
  fseek(f, 0, SEEK_SET);

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

  snprintf(savePath, sizeof(savePath), "%s.sav", romPath);

  CartridgeFileIO fileIO = 
  {
    .saveRamToFile       = fileSaveRam,
    .loadRamFromFile     = fileLoadRam,
    .getExpectedSaveSize = fileGetExpectedSaveSize
  };

  // - - - 2. Initialize Core Components
  if (!cartridgeInit(&fileIO, romData, (u32)romSize)) 
  {
    FORGE_LOG_FATAL("%s", "Failed to initialize cartridge");
    free(romData);
    return 1;
  }

  platformInit();
  ppuInit();
  dmaInit();
  cpuInit();
  timerInit();


  FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");
  bool running = true;
  SDL_Event event;

  CpuContext* cpu = cpuGetContext();

  while (running) 
  {
    u16 pcAtStart = cpu->registers.programCounter;

    while (SDL_PollEvent(&event)) 
    {
      if (event.type == SDL_EVENT_QUIT) 
      {
        running = false;
        break;
      }
    }
    
    if (cpu->mCycleInInstr == 1 && cpu->state != CPU_STATE_FETCH) 
    {
      char debugInfo[256];
      cpuTraceLineToString(pcAtStart, debugInfo, sizeof(debugInfo));
      FORGE_LOG_DEBUG("%s", debugInfo);
    }

    // - - - B. Tick Components
    cpuStepMCycle();
    dmaStepMCycle();
    timerStepMCycle();
    ppuStepMCycle();
    
    PpuContext* ppu = ppuGetContext();
    static u8 last_ly = 0;
        
    // Trigger on the transition to V-Blank
    if (ppu->ly == 144 && last_ly == 143) 
    {
      platformGetContext()->rendering.renderFrame(ppu->frameBuffer, 160, 144);
    }
    last_ly = ppu->ly;
  }
  free(romData);
  return 0;
}
