/**
 * @file cartridge.h
 * @brief Core cartridge handling for the Game Boy emulator,
*/

#pragma once
#include <common.h>
#include <io/memoryBankController.h>

/**
 * @brief Structure representing the metadata of a Game Boy cartridge,
 * Each cartridge contains a header, located at the address range 0x0100 to 0x014F in the ROM data. 
 * The cartridge header provides the following information about the game and the hardware it expects to run on:
 * - 0100 - 0103: Entry Point (4 bytes)
 *   After displaying the Nintendo logo, the built-in boot ROM jumps to address $0100, 
 *   which should then jump to wherever the game wants to start.
 *   Most games put a no op and then jump to 0150.
 *
 * - 0104 - 0133: Nintendo Logo (48 bytes)
 *   This area contains a image that is displayed when the Game Boy is powered on.
 *   On the original game boy, if your game doesnt match the dump in the boot ROM, the game wont run.
 *   If you put the image without buying a license from Nintendo, you will hear from their lawyers.
 *   Genius copyright way.
 *
 * - 0134-0143: Title (16 bytes)
 *   Contains the title of the game in upper case ASCII in 16 butes. 
 *   Can use this to guess the game.
 *
 * - 013F-0142: Manufacture code (4 bytes), 
 *   @see CGBTitleInfo struct
 *   I dont know what this does. 
 *
 * - 0143: CGB flag (1 byte)
 *   This determines whether this is a Game Boy game or a Game Boy Color Game
 *   If the value is 
 *   - 80: The game supports CGB but is backwards compatible with Game Boy 
 *   - C0: The game works on CGB only
 * 
 * - 0144-0145: New Licensee Code (2 bytes)
 *   This tells who published the game.
 *
 * - 0146: SGB flag (1 byte) : tells whether Super Game Boy is supported
 * - 0147: Cartridge Type (1 byte) : tells what hardware is present on the cartridge. 
 * - 0148: ROM size (1 bytes): tells how big the ROM size.
 * - 0149: RAM size (1 bytes): tells how much RAM is present on the cartridge if any.
 * - 014A: Destination code (1 byte): tells whether this was supposed to be Japan only.
 * - 014B: Old Licensee code (1 byte): useless almost.
 * - 014C: Mask ROM version number (1 byte): specifies version of the Game. Who cares
 * - 014D: Header CHecksum (1 byte): used to calculate a checksum to verify if the ROM is corrupted
 * - 014E-014F: Global Checksum (2 bytes): no game uses this.
*/
typedef struct CartridgeMetadata
{
	u8 		entryPoint[4];			///< 0x100-0x0103
	u8 		nintendoLogo[48];	  ///< 0x0104-0x0133
	union TitleInfo						///< 0x0104-0x0133
	{
		char oldTitle[16];			///< old format
		struct CGBTitleInfo 
		{
			char	title[11];			///< 0x0134-0x013E
			char	manufacture[4]; ///< 0x013F-0x0142
			u8		cgbFlag;				///< 0x0143 
		} cgb;
	} titleInfo;
	char	newLicensee[2];			///< 0x0144-0x0145
	u8		sgbFlag;						///< 0x0146
  u8		cartridgeType;      ///< 0x0147 
  u8		romSize;						///< 0x0148
  u8		ramSize;						///< 0x0149
  u8		destinationCode;		///< 0x014A
  u8		oldLicenseeCode;		///< 0x014B
  u8		maskRomVersion;			///< 0x014C
  u8		headerChecksum;			///< 0x014D
  u16		globalChecksum;			///< 0x014E-0x014F
} CartridgeMetadata; 		// - - - total : 80 bytes
COMPILE_TIME_ASSERT(sizeof(CartridgeMetadata) == 80, "CartridgeMetadata struct must be exactly 80 bytes in size");

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


/**
 * @brief Structure representing the context of a loaded cartridge,
 * This structure holds all the relevant information about the currently loaded cartridge, including:
 *   - The size of the ROM data.
 *   - A pointer to the loaded ROM data in memory.
 *   - A pointer to the cartridge metadata extracted from the ROM data.
 *   - A pointer to allocated external RAM data (if the cartridge has external RAM).
 *   - A pointer to the file I/O operations for loading/saving ROM and RAM data.
 * This context is used by the emulator to manage the cartridge data and perform read/write operations..
*/
typedef struct 
{
	bool							 initialized;				///< Indicates whether the cartridge context has been initialized
  u32                romSize;						///< Total size of ROM data in bytes
  const u8*	   			 romData;						///< Pointer to the loaded ROM data
  CartridgeMetadata* metadata;					///< Pointer to the metadata within romData

  u8*								 externalRamData;		///< Pointer to allocated *external* RAM data (if cartridge has external RAM)
  u32                externalRamSize;   ///< Size of the external RAM in bytes

  MapperType mapperType;
  bool			 hasRam;
  bool			 hasBattery;
  bool			 hasRTC;

  bool ramEnabled;   ///< mapper-controlled RAM gating
  bool ramDirty;     ///< set on any write to RAM when enabled

  u16 romBank;       ///< switchable bank for 0x4000-0x7FFF (meaning depends on mapper)
  u8  ramBank;       ///< active RAM bank (when RAM banked)

  union
  {
    MBC0State mbc0;
    MBC1State mbc1;
    MBC2State mbc2;
    MBC3State mbc3;
    MBC5State mbc5;
  } mapper;

  CartridgeFileIO*   fileIO;						///< Pointer to the file I/O operations for loading/saving ROM and RAM data
} CartContext;

/**
 * @brief Global access function to get the current cartridge context,
 * This function provides access to the current cartridge context.
 * @warning Do not modify the returned context directly,
 * @return Pointer to the current cartridge context.
*/
CartContext* cartridgeGetContext(void);

/**
 * @brief Initializes the cartridge system by loading the ROM data and setting up the cartridge context,
 * @param FILE_IO Pointer to a CartridgeFileIO structure.
 * @param ROM_DATA Pointer to the ROM data to be loaded into memory. This should point to a buffer that contains the entire ROM data for the cartridge. 
 * @param ROM_SIZE Size of the ROM data in bytes. This should match the actual size
 * @see CartridgeFileIO for details on the expected function pointers and their behavior.
 * @return true if the cartridge was initialized successfully, false otherwise.
 * @warning Catrdidge doesnt take ownership of the ROM_DATA pointer, so the caller is responsible for ensuring that the ROM data remains valid for the lifetime of the cartridge context.
*/
bool cartridgeInit(
	const CartridgeFileIO* FILE_IO,
	const u8*							 ROM_DATA,
	const u32							 ROM_SIZE);

/**
 * @brief Reads a byte of data from the cartridge at the specified address,
 * @param ADDRESS The address to read from
 * @return The byte of data read from the cartridge at the specified address.
*/
u8   cartridgeRead(u16 ADDRESS);

/**
 * @brief Writes a byte of data to the cartridge at the specified address,
 * This function is used for writing to external RAM or for triggering special hardware behavior on certain cartridge types.
 * @param ADDRESS The address to write to
 * @param VALUE The byte of data to write to the cartridge at the specified address.
*/
void cartridgeWrite(u16 ADDRESS, u8 VALUE);

/**
 * @brief Retrieves the title of the game from the cartridge metadata
 * @return A pointer to a string containing the title of the game.
*/
const char* cartridgeGetTitle(void);

/**
 * @brief Retrieves the type of the cartridge from the cartridge metadata
 * @return The cartridge type code, which indicates the hardware features present on the cartridge (e.g., MBC type, RAM presence, etc.).
*/
const char* cartridgeGetType(void);

/**
 * @brief Retrieves the ROM size of the cartridge from the cartridge metadata
 * @return The size of the ROM in bytes.
*/
u32 cartridgeGetRomSize(void);

/**
 * @brief Retrieves the RAM size of the cartridge from the cartridge metadata
 * @return The size of the RAM in bytes.
*/
u32 cartridgeGetRamSize(void);


/**
 * @brief Retrieves the manufacturer code from the cartridge metadata
 * @return A pointer to a string containing the manufacturer name.
*/
const char* cartridgeGetManufacturer(void);

/**
 * @brief Prints the cartridge metadata to the console for debugging
 * @warning works only if DEBUG is defined, otherwise it does nothing.
*/
void cartridgePrintMetadata(void);

/// @brief Ticks the cartridge, allowing it to perform any necessary updates or operations based on the current state of the emulator.
void cartridgeTick(void);

/// @brief Unloads the current cartridge, freeing any allocated resources and resetting the cartridge context.
void cartridgeUnload(void);

/// @brief Flushes the cartridge RAM, writing any unsaved data to the file system and clearing the RAM data in memory.
void cartridgeFlushRAM(void);

/**
 * @brief Checks if the cartridge type has a battery for saving game data.
 * @return true if the cartridge type has a battery, false otherwise.
*/
bool cartridgeTypeHasBattery(void);

/**
 * @brief Checks if the cartridge type has a real-time clock (RTC) for time-based game features.
 * @return true if the cartridge type has an RTC, false otherwise.
*/
bool cartridgeTypeHasRTC(void);

/**
 * @brief Detects the mapper type of the cartridge based on the cartridge metadata.
 * The mapper type determines how the cartridge handles memory banking, RAM access, and other hardware features
 * @return The detected MapperType for the cartridge, which indicates the specific hardware behavior and features of the cartridge.
*/
MapperType cartridgeDetectMapperType(void);


// - - - Defines for no magic numbers - - - 

#define CART_CGB_SUPPORTED			  0x80
#define CART_CGB_ONLY						  0xC0
#define CART_OLD_LICENSEE_USE_NEW 0x33
#define CART_READ_OFFSET				  0x100
#define CHECKSUM_ADDR_MIN				  0x0134
#define CHECKSUM_ADDR_MAX				  0x014C 
#define RAM_SIZE_MBC2             512
#define NINTENDO_LOGO_SIZE        48
#define NINTENDO_LOGO_OFFSET      0x0104
#define ROM_BANK_SIZE             0x4000
