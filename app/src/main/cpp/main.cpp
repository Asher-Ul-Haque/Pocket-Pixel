
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

extern "C" {
#include <io/cartridge.h>
#include <cpu/cpu.h>
}

// --------------------------------------------------
// Simple File I/O Implementation
// --------------------------------------------------

static std::string g_savePath;

bool fileLoadRom(u8* ROM_DATA, u64 ROM_SIZE)
{
  // Not used (we already pass ROM directly)
  (void)ROM_DATA;
  (void)ROM_SIZE;
  return false;
}

bool fileSaveRam(const u8* RAM_DATA, u32 RAM_SIZE)
{
  if (g_savePath.empty()) return false;

  std::ofstream file(g_savePath, std::ios::binary);
  if (!file) return false;

  file.write(reinterpret_cast<const char*>(RAM_DATA), RAM_SIZE);
  return true;
}

bool fileLoadRam(u8* RAM_DATA, u32 RAM_SIZE)
{
  if (g_savePath.empty()) return false;

  std::ifstream file(g_savePath, std::ios::binary);
  if (!file) return false;

  file.read(reinterpret_cast<char*>(RAM_DATA), RAM_SIZE);
  return true;
}

u32 fileGetExpectedSaveSize()
{
  return 0; // not strictly needed for now
}

// --------------------------------------------------
// Entry
// --------------------------------------------------

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <rom_file>\n";
    return 1;
  }

  const char* romPath = argv[1];

  // --------------------------------------------------
  // Read ROM file into memory
  // --------------------------------------------------
  std::ifstream file(romPath, std::ios::binary | std::ios::ate);
  if (!file)
  {
    std::cerr << "Failed to open ROM file\n";
    return 1;
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<u8> romData(size);

  if (!file.read(reinterpret_cast<char*>(romData.data()), size))
  {
    std::cerr << "Failed to read ROM file\n";
    return 1;
  }

  // --------------------------------------------------
  // Setup save path (same name + .sav)
  // --------------------------------------------------
  g_savePath = std::string(romPath) + ".sav";

  // --------------------------------------------------
  // Setup FileIO
  // --------------------------------------------------
  CartridgeFileIO fileIO = {};
  fileIO.saveRamToFile       = fileSaveRam;
  fileIO.loadRamFromFile     = fileLoadRam;
  fileIO.getExpectedSaveSize = fileGetExpectedSaveSize;

  // --------------------------------------------------
  // Initialize cartridge
  // --------------------------------------------------
  if (!cartridgeInit(&fileIO, romData.data(), (u32)romData.size()))
  {
    std::cerr << "Failed to initialize cartridge\n";
    return 1;
  }
  cpuInit();


  char line[256];
  while (true)
  {
    cpuTick();
    cpuInstructionToString(line, 256);
    std::cout << line << std::endl;
  }

  return 0;
}
