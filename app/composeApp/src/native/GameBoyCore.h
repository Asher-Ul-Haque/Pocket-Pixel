#pragma once

#include "defines.h"

void startEmulator();
void stopEmulator();
void stepFrame();
void getFrame(u8* FRAME_BUFFER);
void getAudio(u8* AUDIO_BUFFER);
void loadROM(u8* ROM);
void setButton(u8 BUTTON, bool PRESSED);
