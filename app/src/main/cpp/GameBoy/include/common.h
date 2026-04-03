/**
 * @file common.h
 * @brief Common utilities and macros for the project
 * meant to be a way to simply includes by just including this file, and not worrying about the order of includes
*/

#pragma once

#include "../../defines.h"
#include "../../ForgeLibrary/include/logger.h"
#include "../../ForgeLibrary/include/asserts.h"

#ifdef __cplusplus
extern "C" {
#endif


#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

#define BIT_SET(a, n, on) { if (on) a |= (1 << n); else a &= ~(1 << n);}

#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

#ifdef __cplusplus
}
#endif
