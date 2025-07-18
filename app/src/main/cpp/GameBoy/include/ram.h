#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

u8 wram_read(u16 address);
void wram_write(u16 address, u8 value);

u8 hram_read(u16 address);
void hram_write(u16 address, u8 value);


#ifdef __cplusplus
}
#endif
