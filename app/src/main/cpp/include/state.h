/**
 * @file state.h 
 * @brief To save save states of the game
*/
#pragma once
#include <common.h>

/**
 * @brief Serializes the entire emulator state into a newly allocated binary buffer.
 * @param OUT_SIZE A pointer to a u32 that will be populated with the total byte size of the buffer.
 * @return A pointer to the malloc'd binary array. THE CALLER MUST FREE THIS MEMORY.
*/
u8* systemSaveStateToMemory(u32* OUT_SIZE);

/**
 * @brief Restores the emulator state from a binary buffer.
 * @param BUFFER The binary array containing the save state.
 * @param SIZE The size of the buffer in bytes (used for validation).
 * @return true if successful, false if the buffer is invalid or corrupt.
*/
bool systemLoadStateFromMemory(const u8* BUFFER, u32 SIZE);
