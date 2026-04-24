#pragma once
#include <common.h>

/**
 * @file dma.h
 * @brief OAM DMA Controller (0xFF46).
 * Handles the high-speed transfer of sprite data to OAM.
*/

typedef struct 
{
  bool active;     /// True if a DMA transfer is currently in progress
  u16  sourceAddr; /// The source address of the DMA transfer (calculated from the value written to 0xFF46)
  u8   byteIndex;  /// The current byte being transferred
  u8   delay;      /// DMA has a small startup delay                   
} DmaContext;

/// @brief Returns a pointer to the global DMA context from anywhere
DmaContext* dmaGetContext(void);


void dmaInit(void);

/**
 * @brief Start a DMA transfer.
 * @param VALUE The value written to 0xFF46 (Source = VALUE * 0x100).
*/
void dmaStart(u8 source_byte);

/**
 * @brief Tick the DMA controller by ONE M-cycle.
 * Should be called once per CPU M-cycle.
*/
void dmaStepMCycle(void);

/// @brief Returns true if a DMA transfer is currently in progress.
bool dmaIsActive(void);


// - - - Helpers
#define DMA_DELAY_CYCLES    2
#define DMA_TRANSFER_CYCLES 160
#define DMA_OFFSET          0xFE00
