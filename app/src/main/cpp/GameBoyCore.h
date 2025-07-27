#pragma once

#include "GameBoy/include/gamepad.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  A,
  B,
  SELECT,
  START,
  DOUBLE_SPEED,
} Buttons;

FORGE_API void       startEmulator ();
FORGE_API void       stopEmulator  ();
FORGE_API void       getFrame      (u32* FRAME_BUFFER);
FORGE_API void       getDebugFrame (u32* DEBUG_BUFFER);
FORGE_API void       getAudio      (u8*  AUDIO_BUFFER);
FORGE_API void       playAudio     ();
FORGE_API void       setButton     (Buttons BUTTON, bool PRESSED);


#ifdef __cplusplus
}
#endif
