#ifndef __ANDROID__
#include "ForgeLibrary/include/filesystem.h"
#include "ForgeLibrary/include/logger.h"
#include "defines.h"
#include "ForgeLibrary/include/asserts.h"
#include "GameBoyCore.h"
#include "GameBoy/include/cpu.h"
#include "GameBoy/include/cartridge.h"


int main(int argc, char* argv[])
{
  FORGE_LOG_INFO("Starting LocalBridge...");

  if (argc < 2)
  {
    FORGE_LOG_ERROR("Usage: LocalBridge <path_to_rom>");
    return 1;
  }
  const char* romPath = argv[1];
  FORGE_LOG_DEBUG("Opening File %s", romPath);

  File romFile;
  if (!openFile(romPath, FILE_MODE_READ, true, &romFile))
  {
    FORGE_LOG_ERROR("Failed to open ROM file: %s", romPath);
    return 1;
  }

  unsigned char* buffer = nullptr;
  unsigned long long fileSize = 0;

  if (!readAllBytes(&romFile, &buffer, &fileSize))
  {
    FORGE_LOG_ERROR("Failed to read ROM file: %s", romPath);
    closeFile(&romFile);
    return 1;
  }

  closeFile(&romFile);

  FORGE_LOG_DEBUG("ROM loaded successfully (%llu bytes)", fileSize);

  cartridgeLoad(reinterpret_cast<u8*>(buffer), fileSize);
  FORGE_LOG_DEBUG("Opening File %s", argv[1]);

  cpuInit();
  // - - - main loop
  while (true)
  {  cpuTick(); }
}
#endif
