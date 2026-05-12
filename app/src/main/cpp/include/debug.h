#pragma once

#include <common.h>

#define SERIAL_BUFFER_SIZE 256
typedef struct 
{
  u8  SB;
  u8  SC;
  u16 index;
  char string[SERIAL_BUFFER_SIZE];
} DebugContext;

DebugContext* debugGetContext(void);


void serialInit(void);

u8 serialRead(u16 ADDR);


void serialWrite(u16 ADDR, u8 VALUE);
