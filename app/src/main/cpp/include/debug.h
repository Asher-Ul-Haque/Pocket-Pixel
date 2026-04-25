#pragma once

#include <common.h>

/**
 * @file debug.h
 * @brief Debug visualization utilities for emulator diagnostics.
 */

// Toggle debug overlays
void debugToggleTileGrid(void);
void debugToggleSprites(void);
void debugToggleWindow(void);

// Query debug states
bool debugIsTileGridEnabled(void);
bool debugIsSpritesEnabled(void);
bool debugIsWindowEnabled(void);

// Apply overlays to frame buffer (call before rendering)
void debugApplyOverlays(u32* frameBuffer);
