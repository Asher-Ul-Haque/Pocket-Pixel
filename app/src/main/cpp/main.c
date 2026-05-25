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

#define TARGET_FRAME_MS_60FPS  16
#define TARGET_FRAME_MS_120FPS 8

static char savePath[1024];

static const char* getModeName(GameBoyMode MODE)
{
  switch (MODE)
  {
    case MODE_DMG_GAMEBOY:      return "DMG";
    case MODE_CGB_GAMEBOY:      return "CGB (backward compatible)";
    case MODE_CGB_ONLY_GAMEBOY: return "CGB only";
    default:                    return "Unknown";
  }
}

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE)
{
  if (savePath[0] == '\0') return false;
  FILE* file = fopen(savePath, "wb");
  if (!file) return false;
  u64 written = fwrite(RAM_DATA, 1, RAM_SIZE, file);
  fclose(file);
  return written == RAM_SIZE;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE)
{
  if (savePath[0] == '\0') return false;
  FILE* file = fopen(savePath, "rb");
  if (!file) return false;
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

  FORGE_LOG_DEBUG("%s", "[BOOT] Starting Pocket Pixel Execution...");

  const char* romPath = ARGUMENT_VECTOR[1];
  FILE* file = fopen(romPath, "rb");
  if (!file) return 1;

  fseek(file, 0, SEEK_END);
  u64 romSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  u8* romData = (u8*)malloc(romSize);
  if (!romData)
  {
    fclose(file);
    return 1;
  }

  if (fread(romData, 1, (size_t)romSize, file) != romSize)
  {
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

  FORGE_LOG_DEBUG("%s", "[BOOT] Initializing Cartridge...");
  if (!cartridgeInit(&fileIO, romData, (u32)romSize))
  {
    FORGE_LOG_FATAL("%s", "Failed to initialize cartridge");
    free(romData);
    return 1;
  }

  FORGE_LOG_DEBUG("%s", "[BOOT] Initializing Platform Subsystems...");
  platformInit();
  
  FORGE_LOG_DEBUG("%s", "[BOOT] Initializing Hardware Cores...");
  // Note: joypadInit() is omitted here as it's assumed to be handled within hardware boots now,
  // but if your architecture requires explicit init in main, include it. Assuming CPU/PPU handle their own.
  cpuInit();
  ppuInit();
  timerInit();
  serialInit();

  PlatformContext* platform = platformGetContext();

  FORGE_LOG_INFO("%s", "--- POCKET PIXEL STARTING ---");
  FORGE_LOG_INFO("Game: %s", cartridgeGetTitle());
  FORGE_LOG_INFO("Mode: %s", getModeName(cartridgeGetContext()->mode));

  bool running      = true;
  bool paused       = false;
  bool doubleSpeed  = false;
  bool pKeyHeld     = false;
  bool fKeyHeld     = false;

  PpuContext* ppu = ppuGetContext();
  u32 stalledIterations = 0;
  
  FORGE_LOG_DEBUG("%s", "[LOOP] Entering Main Execution Loop.");

  while (running)
  {
    u64 frameStartMs = SDL_GetTicks();

    // The platform poll handles hardware joypad routing natively.
    if (platform && platform->input.poll)
    {
      platform->input.poll(&running);
    }

    if (!running) 
    {
      FORGE_LOG_DEBUG("%s", "[LOOP] Stop requested. Breaking main loop.");
      break;
    }

    // --- UI Toggles (Emulator Shell Control) ---
    const bool* keyboard = SDL_GetKeyboardState(NULL);

    bool pPressed = keyboard[SDL_SCANCODE_P];
    if (pPressed && !pKeyHeld)
    {
      paused = !paused;
      FORGE_LOG_INFO("Emulation %s", paused ? "PAUSED" : "RESUMED");
    }
    pKeyHeld = pPressed;

    bool fPressed = keyboard[SDL_SCANCODE_F];
    if (fPressed && !fKeyHeld)
    {
      doubleSpeed = !doubleSpeed;
      FORGE_LOG_INFO("Speed Mode: %s", doubleSpeed ? "2x (120 FPS)" : "Normal (60 FPS)");
    }
    fKeyHeld = fPressed;

    if (!paused)
    {
      u32 syncCycleFailsafe = 0;

      while (running && !ppu->frameReady)
      {
        cpuTick();

        if (cpuGetContext()->stopped)
        {
          if (ppu->registers.key1 & 0x01)
          {
            ppuExecuteSpeedSwitch();
            cpuGetContext()->stopped = false;
          }
        }

        timerStepMCycle();

        u8 dotsToTick = (ppu->registers.key1 & 0x80) ? 2 : 4;
        for (u8 i = 0; i < dotsToTick; ++i)
        {
          ppuTick();
        }

        syncCycleFailsafe++;
        if (syncCycleFailsafe > 1000000)
        {
            FORGE_LOG_DEBUG("[TRAP] CPU/PPU executed 1,000,000 times without frameReady. LCDC: 0x%02X, LY: %u, Mode: %u", ppu->registers.lcdc, ppu->registers.ly, ppu->mode);
            syncCycleFailsafe = 0; 
        }
      }

      if (!ppu->frameReady)
      {
        stalledIterations++;
        if ((stalledIterations % 120) == 0)
        {
          FORGE_LOG_WARNING(
            "[RENDER] frameReady not reached: lcdEnabled=%d ly=%u mode=%u dot=%u lcdc=0x%02X",
            (ppu->registers.lcdc & 0x80) ? 1 : 0,
            ppu->registers.ly,
            (u32)ppu->mode,
            ppu->dotCount,
            ppu->registers.lcdc
          );
        }
        continue;
      }

      stalledIterations = 0;
      ppu->frameReady = false;
    }

    if (platform)
    {
      if (platform->video.renderFrame)
      {
        platform->video.renderFrame(&ppu->frameBuffer);
      }

      if (platform->video.present)
      {
        platform->video.present();
      }
    }

    const u32 targetFrameMs = doubleSpeed ? TARGET_FRAME_MS_120FPS : TARGET_FRAME_MS_60FPS;
    u64 frameEndMs = SDL_GetTicks();
    u64 frameDurationMs = frameEndMs - frameStartMs;

    if (frameDurationMs < targetFrameMs)
    {
      SDL_Delay((u32)(targetFrameMs - frameDurationMs));
    }
  }

  FORGE_LOG_DEBUG("%s", "[SHUTDOWN] Exiting emulator sequence.");
  if (platform && platform->video.cleanup)
  {
    platform->video.cleanup();
  }

  free(romData);
  return 0;
}
