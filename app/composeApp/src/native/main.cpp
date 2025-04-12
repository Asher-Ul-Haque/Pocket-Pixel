#ifndef __ANDROID__
#include "ForgeLib/include/logger.h"
#include "GameBoyCore.h"
#include "Machine/Parts/cartridge.h"

int main(int argc, char const *argv[])
{
	startEmulator();
	cartridgeLoad(nullptr, 0);
	FORGE_LOG_INFO("Hello World");
	return 0;
}
#endif