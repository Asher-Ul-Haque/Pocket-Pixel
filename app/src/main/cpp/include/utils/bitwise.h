/**
 * @file bitwise.h
 * @brief Utility macros for common bitwise operations and simple range checks.
 * @note Taken from Forge Library, 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
*/

#pragma once

/**
 * @brief Returns the value of the n-th bit of a.
 *
 * Evaluates to `1` if the bit is set, otherwise `0`.
 *
 * @param a Value whose bit will be inspected.
 * @param n Bit index (starting from 0).
*/
#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

/**
 * @brief Sets the n-th bit of a.
 *
 * @param a Value whose bit will be modified.
 * @param n Bit index (starting from 0).
*/
#define BIT_SET(a, n)   ((a) |=  (1U << (n)))

/**
 * @brief Clears the n-th bit of a.
 *
 * @param a Value whose bit will be modified.
 * @param n Bit index (starting from 0).
*/
#define BIT_RESET(a, n) ((a) &= ~(1U << (n)))

/**
 * @brief Checks whether a value lies within a closed interval.
 *
 * @param a Value to test.
 * @param b Lower bound.
 * @param c Upper bound.
 *
 * @return True if `b <= a <= c`.
*/
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))
