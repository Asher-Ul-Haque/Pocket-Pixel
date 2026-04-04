/**
 * @file dataTypes.h
 * @brief Common fixed-width primitive type aliases used throughout the utils module.
 *
 * Provides short aliases for signed/unsigned integers and floating-point
 * types to simplify usage and improve readability.
 * @note Taken from Forge Library, 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
 */

#pragma once
#include <stdint.h>

// - - - Unsigned integer types - - - 

typedef uint8_t  u8;  ///< 8-bit unsigned integer type.
typedef uint16_t u16; ///< 16-bit unsigned integer type.
typedef uint32_t u32; ///< 32-bit unsigned integer type.
typedef uint64_t u64; ///< 64-bit unsigned integer type.

// - - - Signed integer types - - - 

typedef int8_t  i8;  ///< 8-bit signed integer type.
typedef int16_t i16; ///< 16-bit signed integer type.
typedef int32_t i32; ///< 32-bit signed integer type.
typedef int64_t i64; ///< 64-bit signed integer type.


// - - - Floating-point types - - - 

typedef float f32;  ///< 32-bit single-precision floating-point type.
typedef double f64; ///< 64-bit double-precision floating-point type.
