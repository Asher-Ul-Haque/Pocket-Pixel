#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
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
#include <joypad.h>

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
    FORGE_LOG_ERROR(
      "Failed to open save file for reading: %s. Starting with empty RAM.",
      savePath
    );
    return false;
  }

  u64 read = fread(RAM_DATA, 1, RAM_SIZE, file);
  fclose(file);
  return read == RAM_SIZE;
}

u32 fileGetExpectedSaveSize(void)
{
  return 0;
}

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

  if (platform && platform->input.printKeybinds)
  {
    platform->input.printKeybinds();
  }

  FORGE_LOG_INFO("%s", "Hotkeys:");
  FORGE_LOG_INFO("%s", "P = Pause/Resume");
  FORGE_LOG_INFO("%s", "F = Toggle 2x Speed (120 FPS)");

  bool running = true;
  bool paused = false;
  bool doubleSpeed = false;

  bool pKeyHeld = false;
  bool fKeyHeld = false;

  PpuContext* ppu = ppuGetContext();

  #define TARGET_FRAME_MS_60FPS 16
  #define TARGET_FRAME_MS_120FPS 8

  u32 stalledIterations = 0;

  while (running)
  {
    u64 frameStartMs = SDL_GetTicks();

    // Poll platform input
    if (platform && platform->input.poll)
    {
      platform->input.poll(&running);
    }

    if (!running)
    {
      break;
    }

    // Keyboard state
    const bool* keyboard = SDL_GetKeyboardState(NULL);

    // Toggle Pause (P)
    bool pPressed = keyboard[SDL_SCANCODE_P];
    if (pPressed && !pKeyHeld)
    {
      paused = !paused;

      FORGE_LOG_INFO(
        "Emulation %s",
        paused ? "PAUSED" : "RESUMED"
      );
    }
    pKeyHeld = pPressed;

    // Toggle Double Speed (F)
    bool fPressed = keyboard[SDL_SCANCODE_F];
    if (fPressed && !fKeyHeld)
    {
      doubleSpeed = !doubleSpeed;

      FORGE_LOG_INFO(
        "Speed Mode: %s",
        doubleSpeed ? "2x (120 FPS)" : "Normal (60 FPS)"
      );
    }
    fKeyHeld = fPressed;

    // ----------------------------------------
    // Emulation Step
    // ----------------------------------------
    if (!paused)
    {

      while (
        running &&
        !ppu->frameReady)
      {
        cpuTick();
        if (cpuGetContext()->stopped)
        {
          if (ppuGetContext()->registers.doubleSpeed & 0x01)
          {
            ppuExecuteSpeedSwitch();
            cpuGetContext()->stopped = false;
          }
        }

        timerStepMCycle();
        ppuTick();

      }

      if (!ppu->frameReady)
      {
        stalledIterations++;

        if ((stalledIterations % 120) == 0)
        {
          FORGE_LOG_WARNING(
            "[RENDER] frameReady not reached: "
            "lcdEnabled=%d ly=%u mode=%u dot=%u lcdc=0x%02X",
            ppuIsLcdEnabled() ? 1 : 0,
            ppu->registers.ly,
            (u32)ppu->mode,
            ppu->dotCount,
            ppu->registers.lcdc
          );
        }

        continue;
      }

      stalledIterations = 0;

      // Build completed frame
      ppuRenderFrame();
      ppu->frameReady = false;
    }

    // ----------------------------------------
    // Presentation Phase
    // Still renders while paused
    // ----------------------------------------
    if (platform)
    {
      if (platform->rendering.renderFrame)
      {
        platform->rendering.renderFrame(&ppu->currentFrame);
      }

      if (platform->rendering.drawTileView)
      {
        platform->rendering.drawTileView(
          ppu->vram[0],
          ppu->vram[1]
        );
      }

      if (platform->rendering.drawMapView)
      {
        platform->rendering.drawMapView(
          ppu->vram[0],
          ppu->vram[1],
          0
        );
      }

      if (platform->rendering.present)
      {
        platform->rendering.present();
      }
    }

    // ----------------------------------------
    // FPS Cap
    // ----------------------------------------
    const u32 targetFrameMs =
      doubleSpeed
      ? TARGET_FRAME_MS_120FPS
      : TARGET_FRAME_MS_60FPS;

    u64 frameEndMs = SDL_GetTicks();
    u64 frameDurationMs = frameEndMs - frameStartMs;

    if (frameDurationMs < targetFrameMs)
    {
      SDL_Delay((u32)(targetFrameMs - frameDurationMs));
    }
  }

  if (platform && platform->rendering.cleanup)
  {
    platform->rendering.cleanup();
  }

  free(romData);

  return 0;
}
