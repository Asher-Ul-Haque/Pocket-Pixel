#pragma once 

/**
 * @file platform.h
 * @brief Platform-specific definitions and utilities.
 * @note a Platform must provide the following:
 * - `PLATFORM_NAME`: A string literal representing the platform name.
 * - `FILE_IO` : Support for file input/output operations.
 * - `RENDERING`: Support for rendering graphics.
 * - `INPUT`: Support for handling user input.
 * - `AUDIO`: Support for audio playback and processing.
 * - `NETWORKING`: Support for network communication.
*/

#include <ppu/ppu.h>
#include <common.h>

/**
 * @brief Structure representing the file I/O operations for cartridge data,
 * This structure defines function pointers for 
 *   - loading ROM data, 
 *   - saving and loading RAM data
 *   - getting the expected size of the save data for the cartridge.
 * This allows the cartridge code to be decoupled from the specific file handling implementation,
*/
typedef struct CartridgeFileIO 
{
  /**
   * @brief Function pointer for saving RAM data to a file.
   * @param RAM_DATA Pointer to the RAM data that should be saved.
   * @param RAM_SIZE Size of the RAM data in bytes.
   * @return true if the RAM data was saved successfully, false otherwise.
  */
  bool (*saveRamToFile)   (const u8* RAM_DATA, u32 RAM_SIZE);

  /**
   * @brief Function pointer for loading RAM data from a file.
	 * @param RAM_DATA_BUFFER Pointer to a buffer where the RAM data should be loaded.
	 * @param BUFFER_SIZE Size of the buffer in bytes.
	 * @return true if the RAM data was loaded successfully, false otherwise.
	*/
  bool (*loadRamFromFile) (u8* RAM_DATA_BUFFER, u32 BUFFER_SIZE);

  /**
   * @brief Function pointer for getting the expected size of the save data for the cartridge.
	 * This is used to determine how much RAM data to allocate and how much to read/write when saving/loading.
	 * @return The expected size of the save data in bytes.
	*/
  u32  (*getExpectedSaveSize)(void);
} CartridgeFileIO;


typedef struct Renderer
{
  void (*init)(void);   ///< Set up renderer 

  void (*renderFrame) (const PpuFrame* FRAME); ///< The main screen 

  void (*drawTileView) (const u8* VRAM_BANK_0, const u8* VRAM_BANK_1);
  void (*drawMapView)  (const u8* VRAM_BANK_0, const u8* VRAM_BANK_1, u8 MAP_SELECT);

  void (*present) (void);
  void (*cleanup) (void);
} Renderer;

typedef struct InputHandler
{
  void (*poll)         (bool* RUNNING);
  void (*printKeybinds)(void);
} InputHandler;

typedef struct AudioSystem
{
  u8 empty; /// TODO
} AudioSystem;

typedef struct NetworkingSystem
{
  u8 empty; /// TODO
} NetworkingSystem;

typedef struct PlatformContext
{
  const char*       name;       /// A string literal representing the platform name.
  CartridgeFileIO   fileIO;     /// Support for file input/output operations.
  Renderer          rendering;  /// Support for rendering graphics.
  InputHandler      input;      /// Support for handling user input.
  AudioSystem       audio;      /// Support for audio playback and processing.
  NetworkingSystem  networking; /// Support for network communication.
} PlatformContext;

PlatformContext* platformGetContext(void);
void             platformInit(void);
