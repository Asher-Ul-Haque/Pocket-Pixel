#pragma once
#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif

u8 io_read(u16 address);
void io_write(u16 address, u8 value);

#ifdef __cplusplus
}
#endif
