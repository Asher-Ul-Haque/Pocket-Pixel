/**
 * @file serial.h
 * @brief Serial Data Transfer (Link Cable) implementation
*/
#pragma once
#include <common.h>

// - - - MACROS & REGISTERS - - -
#define ADDR_SB 0xFF01 ///< Serial transfer data register
#define ADDR_SC 0xFF02 ///< Serial transfer control register

#define SC_TRANSFER_ENABLE_BIT  0x80 ///< Bit 7: 1 = Transfer in progress/requested
#define SC_CLOCK_SPEED_BIT      0x02 ///< Bit 1: 1 = Fast (CGB only), 0 = Normal
#define SC_CLOCK_SELECT_BIT     0x01 ///< Bit 0: 1 = Internal (Master), 0 = External (Slave)

#define SB_INIT_VALUE 0x00 
#define SC_INIT_VALUE 0x7E 

#define SC_MASK 0x7E 

/// @brief The serial context for link cable 
typedef struct SerialContext
{
  u8   sb;             ///< The 8-bit payload buffer
  u8   sc;             ///< The control register
  bool isTransferring; ///< Internal lock to prevent overlapping network requests
} SerialContext;

// - - - API - - -

/**
 * @brief Global access to the serial context 
 * @return global serial state 
 * @note This will never return a null pointer
*/
SerialContext* serialGetContext(void);

/// @brief Initialize the link cable system 
void serialInit(void);

/**
 * @brief Bus access to the serial sttate 
 * @param ADDRESS the address to read from 
 * @return the value at the given address
*/
u8 serialRead(u16 ADDRESS);

/**
 * @brief Bus access to the serial sttate 
 * @param ADDRESS the address to write to 
 * @param VALUE what to write
*/
void serialWrite(u16 ADDRESS, u8 VALUE);

/**
 * @brief Platform will call this function to complete the transaction when network data arrives.
 * @param INCOMING_BYTE the byte to be recieved by the gameboy
*/
void coreCompleteSerialTransfer(u8 INCOMING_BYTE);
