#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef __ANDROID__
#include <jni.h>
#endif
#ifdef __cplusplus
#include <string>
extern "C" {
#endif



// - - - Data Types - - -

// - - - Unsigned int types.
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

// - - - Signed int types.
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

// - - - Floating point types
typedef float       f32;
typedef double      f64;


// - - - Asserts - - -

// - - - Properly define static assertions.
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

// - - - Ensure all types are of the correct size.
#if defined(__clang__)
STATIC_ASSERT(sizeof(u8)  == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8)  == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");
#endif


// - - - Import Export - - -

// - - - export
#ifdef FORGE_EXPORT
#ifdef _MSC_VER
    #define FORGE_API __declspec(dllexport)
  #else
    #define FORGE_API __attribute__((visibility("default")))
  #endif
#else

// - - - Imports
#ifdef _MSC_VER
#define FORGE_API __declspec(dllimport)
#else
#define FORGE_API
#endif
#endif

// - - - for debuggers - - -
#if _MSC_VER
#include <intrin.h>
  #define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

#define FORGE_CLAMP(VALUE, MIN, MAX) (VALUE <= MIN) ? MIN : (VALUE >= MAX) ? MAX : VALUE;

// - - - Inlining
#if defined(__clang__) || defined(__gcc__)
#define FORGE_INLINE __attribute__((always_inline)) inline
#define FORGE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define FORGE_INLINE __forceinline
  #define FORGE_NOINLINE __declspec(noinline)
#else
#define FORGE_INLINE static inline
#define FORGE_NOINLINE
#endif
#ifdef __cplusplus
}
#endif