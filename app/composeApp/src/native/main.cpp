#ifndef __ANDROID__
#include "ForgeLib/include/logger.h"
#include "GameBoyCore.h"

int main(int argc, char const *argv[])
{
	startEmulator();
	loadCartridge(nullptr, 0);
	FORGE_LOG_INFO("Hello World");
	return 0;
}
#endif