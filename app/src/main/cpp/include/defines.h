/**
 * @file defines.h
 * @brief central place for all global defines and macros, such as export, asserts, logging 
 * @note this file should be included in all public headers, and should be the first include in those headers
 * @note if you want to change any configurations, such as enabling asserts, or changing the log format, this is the file you should edit.
 * @warning if you do edit this file, make sure to not include any other library headers, as that would cause a circular dependency
 * @note Taken from Forge Library,
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
*/

#pragma once
#include <stdarg.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif


// - - - | Config | - - - 

// - - - asserts 
/**
 * @brief enables asserts and todos 
 * @see asserts.h
*/
#ifndef FORGE_ASSERTS_ENABLED
  #define FORGE_ASSERTS_ENABLED 1
#endif


// - - - logging - - - 
/**
 * @brief enables printing log colors, 
 * @warning if you are planning on piping the output to a file, consider turning this off
 * @see logger.h
*/
#ifndef PRINT_LOG_COLORS
  #define PRINT_LOG_COLORS      1
#endif

/**
 * @brief enables printing log types
 * @see logger.h
*/
#ifndef PRINT_LOG_TYPES
  #define PRINT_LOG_TYPES       1
#endif

#ifdef __cplusplus
}
#endif
