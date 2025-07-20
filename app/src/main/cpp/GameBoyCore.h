#pragma once

#include "defines.h"


FORGE_API void       startEmulator (u32* FRAME_BUFFER);
FORGE_API void       stopEmulator  ();
FORGE_API void       getFrame      (u32* FRAME_BUFFER);
FORGE_API void       getDebugFrame (u32* DEBUG_BUFFER);
FORGE_API void       getAudio      (u8* AUDIO_BUFFER);
FORGE_API void       setButton     (u8 BUTTON, bool PRESSED);
