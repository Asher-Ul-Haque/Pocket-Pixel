#ifndef __ANDROID__
#include "ForgeLib/include/logger.h"
#include "GameBoyCore.h"
#include "Machine/Parts/emu.h"
#include "Machine/Parts/cartridge.h"

int main(int argc, char const *argv[])
{
    emuRunning(nullptr, 0);
	FORGE_LOG_INFO("EMU STARTED");
	return 0;
}
#endif