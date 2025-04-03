#pragma once

#include "defines.h"
#include "Machine/Parts/cart.h"
#include "Machine/Parts/cpu.h"

typedef struct GBContext
{
	bool 	paused;
	bool 	running;
	u64 	ticks;
} GBContext;

FORGE_API GBContext* 	getContext();
FORGE_API void 			startEmulator();
FORGE_API void 			stopEmulator();
FORGE_API void 			stepFrame();
FORGE_API void 			getFrame(u8* FRAME_BUFFER);
FORGE_API void 			getAudio(u8* AUDIO_BUFFER);
FORGE_API void 			setButton(u8 BUTTON, bool PRESSED);