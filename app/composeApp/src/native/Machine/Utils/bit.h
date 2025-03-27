#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - This file contains bitwise functions for help

// - - - check if the n'th bit of an integer is high or low
#define IS_BIT_SET(NUMBER, BIT_POSITION) ((NUMBER & (1 << BIT_POSITION)) ? true : false); 
// - - - set the n'th bit of an integer to on 
#define SET_BIT(NUMBER, BIT_POSITION, ON) (ON ? (NUMBER) |= (1 << BIT_POSITION) : (NUMBER) &= ~(1 << BIT_POSITION));


#ifdef __cplusplus
}
#endif
