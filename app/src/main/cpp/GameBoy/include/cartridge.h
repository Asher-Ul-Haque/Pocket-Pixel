#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "mappers.h"

#ifdef __cplusplus
extern "C" {
#endif

// - - - What is a cartridge - - - 

typedef struct CartridgeMetadata
{
	u8 		entry[4];						// - - - 4 bytes
	u8 		logo[48];						// - - - 48 bytes
	char 	title[16];					// - - - 16 bytes
	u16 	newLicenseCode;			// - - - 2 bytes
	u8 		sgbFlag;						// - - - 1 byte
	u8 		type;								// - - - 1 byte (This byte determines the MBC type!)
	u8 		romSize;						// - - - 1 byte
	u8 		ramSize;						// - - - 1 byte
	u8 		destinationCode;		// - - - 1 byte
	u8 		licenseCode;				// - - - 1 byte
	u8 		version;						// - - - 1 byte
	u8 		checksum;						// - - - 1 byte
	u16 	globalChecksum;			// - - - 2 bytes
} CartridgeMetadata; 		// - - - total : 80 bytes

// - - - Cartridge Context Structure 
typedef struct 
{
  char               filename[1024];
  u32                romSize;						// - - - Total size of ROM data in bytes
  u8*								 romData;						// - - - Pointer to the loaded ROM data
  CartridgeMetadata* metadata;					// - - - Pointer to the metadata within romData

  u8*								 externalRamData;		// - - - Pointer to allocated *external* RAM data (if cartridge has external RAM)
  u32                externalRamSize;

  MapperType         mapperType;				// - - - The determined MBC type

  // - - - Union for MBC-specific state
  union 
  {
    MBC0State       mbc0;
    MBC1State       mbc1;
    MBC2State       mbc2;
    MBC3State       mbc3;
    MBC5State       mbc5;
  } mapperState;
} CartContext;

FORGE_API CartContext* cartridgeGetContext();


// - - - Cartridge Functions - - -

// - - - load from file
FORGE_API bool cartridgeLoad(u8* CARTRIDGE, u64 SIZE);

// - - - read and write from the cartridge
FORGE_API u8   cartridgeRead(u16 ADDRESS);
FORGE_API void cartridgeWrite(u16 ADDRESS, u8 VALUE);

// - - - ram 
FORGE_API u32  cartridgeSaveRAM(u8* BUFFER, u32 BUFFER_SIZE);
FORGE_API bool cartridgeLoadRAM(u8* BUFFER, u32 BUFFER_SIZE);

// - - - tick 
FORGE_API void cartridgeTickRTC();

// - - - Cleanup function (important for freeing allocated RAM)
FORGE_API void cartridgeUnload();

#ifdef __cplusplus
}
#endif
