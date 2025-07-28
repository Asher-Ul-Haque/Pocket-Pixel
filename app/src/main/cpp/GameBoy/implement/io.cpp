#include "../include/io.h"
#include "../include/gamepad.h" // For gamepadRead, gamepadWrite
#include "../include/timer.h"   // For timerRead, timerWrite
#include "../include/cpu.h"     // For cpuGetInterruptFlags, cpuSetInterruptFlags
#include "../include/apu.h"     // For apuRead, apuWrite
#include "../include/ppu.h"     // Include new PPU header
#include "../../ForgeLibrary/include/logger.h" // For FORGE_LOG_ERROR
#include "../include/common.h"

static char serialData[2]; // Assuming this is defined globally in io.cpp

u8 ioRead(u16 ADDRESS)
{
    if (ADDRESS == 0xFF00)             return gamepadRead();
    if (ADDRESS == 0xFF01)             return serialData[0];
    if (ADDRESS == 0xFF02)             return serialData[1];
    if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) return timerRead(ADDRESS);
    if (ADDRESS == 0xFF0F)             return cpuGetInterruptFlags();
    if (BETWEEN(ADDRESS, 0xFF40, 0xFF4B)) return ppuRead(ADDRESS); // Use new ppuRead
    if (BETWEEN(ADDRESS, 0xFF10, 0xFF26) || (BETWEEN(ADDRESS, 0xFF30, 0xFF3F))) 
        return apuRead(ADDRESS);
    
    if (ADDRESS == 0xFF4F)
    {
        return 0xFF; // - - - no gameboy color (VRAM Bank Select, reads FF on DMG)
    }
    if (ADDRESS == 0xFF50) { // Boot ROM disable register
        // This register's read behavior depends on if the boot ROM is mapped.
        // For simplicity, assuming it's handled elsewhere or returns default.
        return 0xFF;
    }
    if (ADDRESS == 0xFF7F) { // Unused I/O registers
        return 0xFF;
    }

    FORGE_LOG_ERROR("UNSUPPORTED ioRead(%04X)\n", ADDRESS);
    return 0;
}

void ioWrite(u16 ADDRESS, u8 VALUE) 
{
    if (ADDRESS == 0xFF00)
    {
        gamepadWrite(VALUE);
        return;
    }

    if (ADDRESS == 0xFF01)
    {
        serialData[0] = VALUE;
        return;
    }

    if (ADDRESS == 0xFF02) 
    {
        serialData[1] = VALUE;
        return;
    }

    if (BETWEEN(ADDRESS, 0xFF04, 0xFF07)) 
    {
        timerWrite(ADDRESS, VALUE);
        return;
    }
        
    if (ADDRESS == 0xFF0F) 
    {
        cpuSetInterruptFlags(VALUE);
        return;
    }
    
    if (BETWEEN(ADDRESS, 0xFF40, 0xFF4B))
    {
        ppuWrite(ADDRESS, VALUE); // Use new ppuWrite
        return;
    }

    if (BETWEEN(ADDRESS, 0xFF10, 0xFF26) || (BETWEEN(ADDRESS, 0xFF30, 0xFF3F))) 
    { 
        apuWrite(ADDRESS, VALUE);
        return;
    }

    if (ADDRESS == 0xFF4F)
    {
        // This is VRAM Bank Select for CGB, write is ignored on DMG.
        return; 
    }
    if (ADDRESS == 0xFF50) { // Boot ROM disable register
        // Writing 0x01 here disables the boot ROM.
        // The effect of this write should be handled by the CPU/Bus.
        // For now, we'll just acknowledge the write.
        return;
    }
    if (ADDRESS == 0xFF7F) { // Unused I/O registers
        return;
    }

    FORGE_LOG_ERROR("UNSUPPORTED ioWrite(%04X) = %02X\n", ADDRESS, VALUE);
}
