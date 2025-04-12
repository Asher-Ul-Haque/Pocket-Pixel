#include "ram.h"
#include "../../ForgeLib/include/logger.h"

typedef struct
{
    u8 wram[0x2000];
    u8 hram[0x80];
} ramCTX;
// - - - ram_context;

static ramCTX ctx;

// - - - WRAM IMPLEMENTATION
u8 wramRead(u16 address)
{
    address-= 0xC000;
    if(address >= 0x2000)
    {
        FORGE_LOG_ERROR("INVALID WRAM READ AT %08X", address+ 0xC000);
        exit(-1);
    }
    return ctx.wram[address];
}

void wramWrite(u16 address, u8 value)
{
    address-=0xC000;
    ctx.wram[address] = value;

}

// - - - HRAM IMPLEMENTATION
u8 hramRead(u16 address)
{
    address-= 0xFF80;
    return ctx.hram[address];
}

void hramWrite(u16 address, u8 value)
{
    address-=0xFF80;
    ctx.hram[address] = value;

}