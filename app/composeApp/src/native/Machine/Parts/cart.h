#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - What the Catridge is - - - 

typedef struct CartridgeMetadata
{
	u8 		entry[4]; 		// - - - 4  bytes 
	u8 		logo[48]; 		// - - - 48 bytes
	char 	title[16]; 		// - - - 16 bytes
	u16 	newLicenseCode; // - - - 2  bytes
	u8 		sgbFlag; 		// - - - 1  byte
	u8 		type; 			// - - - 1  byte
	u8 		romSize; 		// - - - 1  byte
	u8 		ramSize; 		// - - - 1  byte
	u8 		destinationCode;// - - - 1  byte
	u8 		licenseCode; 	// - - - 1  byte
	u8 		version; 		// - - - 1  byte
	u8 		checksum; 		// - - - 1  byte
	u16 	globalChecksum; // - - - 2  bytes
} CartridgeMetadata; 		// - - - total : 80 bytes

FORGE_API bool 			loadCartridge(u8* CARTRIDGE, u64 SIZE);


#ifdef __cplusplus
}
#endif
