#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <common.h>
#include <timer.h>
#include <platform.h>
#include <cpu/cpu.h>
#include <ppu/ppu.h>
#include <cartridge/cartridge.h>
#include <debug.h>
#include <input/joypad.h>

#define DOTS_PER_FRAME  70224
#define DOTS_PER_MCYCLE 4

static char savePath[1024];

static const char* getModeName(GameBoyMode mode)
{
  switch (mode)
  {
    case MODE_DMG_GAMEBOY:      return "DMG";
    case MODE_CGB_GAMEBOY:      return "CGB (backward compatible)";
    case MODE_CGB_ONLY_GAMEBOY: return "CGB only";
    default: return "Unknown";
  }
}

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE)
{
  if (savePath[0] == '\0')
  {
    FORGE_LOG_ERROR("%s", "Save path is not set. Cannot save RAM.");
    return false;
  }

  FILE* file = fopen(savePath, "wb");
  if (!file)
  {
    FORGE_LOG_ERROR("Failed to open save file for writing: %s", savePath);
    return false;
  }

  u64 written = fwrite(RAM_DATA, 1, RAM_SIZE, file);
  fclose(file);
  return written == RAM_SIZE;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE)
{
  if (savePath[0] == '\0')
  {
    FORGE_LOG_ERROR("%s", "Save path is not set. Cannot load RAM.");
    return false;
  }

  FILE* file = fopen(savePath, "rb");
  if (!file)
  {
    FORGE_LOG_ERROR("Failed to open save file for reading: %s. Starting with empty RAM.", savePath);
    return false;
  }

  u64 read = fread(RAM_DATA, 1, RAM_SIZE, file);
  fclose(file);
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
  FILE* file = fopen(romPath, "rb");
  if (!file)
  {
    FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  u64 romSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  u8* romData = (u8*)malloc(romSize);
  if (!romData)
  {
    FORGE_LOG_FATAL("%s", "Memory allocation failed for ROM data");
    fclose(file);
    return 1;
  }

  const u64 bytesRead = (u64)fread(romData, 1, (size_t)romSize, file);
  if (bytesRead != romSize)
  {
    FORGE_LOG_ERROR("%s", "Failed to read ROM data");
    free(romData);
    fclose(file);
    return 1;
  }

  fclose(file);
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
  joypadInit();
  cpuInit();
  ppuInit();
  timerInit();
  serialInit();

  PlatformContext* platform = platformGetContext();

  FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");
  FORGE_LOG_INFO("Game: %s", cartridgeGetTitle());
  FORGE_LOG_INFO("Mode: %s", getModeName(cartridgeGetContext()->mode));
  if (platform && platform->input.printKeybinds) platform->input.printKeybinds();

  bool running = true;
  while (running)
  {
    if (platform && platform->input.poll) platform->input.poll(&running);

    for (u32 frameCycles = 0; frameCycles < DOTS_PER_FRAME; )
    {
      cpuTick();
      timerStepMCycle();
      ppuTick(DOTS_PER_MCYCLE);
      frameCycles += DOTS_PER_MCYCLE;
    }

    if (platform && platform->rendering.present)
    {
      platform->rendering.present();
    }
  }

  if (platform && platform->rendering.cleanup) platform->rendering.cleanup();
  free(romData);
  return 0;
}
