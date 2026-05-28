/**
 * @file logger.h
 * @brief Simple colored console logging macros 
 *
 * Provides lightweight logging for console output with optional color formatting
 * Behavior changes depending on whether "DEBUG" is defined or not.
 * When "DEBUG" is not defined, only FATAL and ERROR logs are enabled, while WARNING, INFO, DEBUG, and TRACE logs are disabled. 
 * Log levels:
 * - FATAL: Critical error (bright red)
 * - ERROR: Recoverable error (red)
 * - WARNING: Noncritical warning
 * - INFO: Informational message
 * - DEBUG: Debugging output
 * - TRACE: Verbose tracing information
 * @note Taken from Forge Library, 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
*/
#pragma once 
#include <defines.h>
#ifdef __cplusplus
extern "C" {
#endif


// - - - | Log macros and defs | - - - 


// - - - Log Levels - - -

/// @brief Log levels for categorizing log messages, used by the logOutput function to determine the severity of the message and how it should be displayed
typedef enum LogLevel 
{
  LOG_LEVEL_FATAL   =   0, ///< Just give up and die 
  LOG_LEVEL_ERROR   =   1, ///< Something went wrong badly
  LOG_LEVEL_WARNING =   2, ///< Are you sure about that
  LOG_LEVEL_INFO    =   3, ///< Just some information
  LOG_LEVEL_DEBUG   =   4, ///< Debugging information
  LOG_LEVEL_TRACE   =   5  ///< Trace every step
} LogLevel;


// - - - API Controls - - -

/** 
 * @brief Core logging function, takes a log level and a message with optional formatting arguments
 * @param LEVEL The log level (e.g., LOG_LEVEL_ERROR)
 * @param MESSAGE The log message format string (like printf)
 * @param ... Optional additional arguments for formatting the message
 * @warning This function is intended to be used through the provided macros (e.g., FORGE_LOG_ERROR) rather than called directly. The macros will handle enabling/disabling log levels based on the build configuration (e.g., DEBUG) and will call this function with the appropriate log level and message.
 * @see FORGE_LOG_FATAL, FORGE_LOG_ERROR, FORGE_LOG_WARNING, FORGE_LOG_INFO, FORGE_LOG_DEBUG, FORGE_LOG_TRACE for examples of how to use this function through macros.
*/
void logOutput(LogLevel LEVEL, const char* MESSAGE, ...); // - - - Multivariate, takes any number of arguments greater than 1

// - - - Fatal log
#ifndef FORGE_LOG_FATAL
  #define FORGE_LOG_FATAL(MESSAGE, ...) logOutput(LOG_LEVEL_FATAL, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#endif

#ifndef FORGE_LOG_ERROR
  #define FORGE_LOG_ERROR(MESSAGE, ...) logOutput(LOG_LEVEL_ERROR, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#endif

#if !defined(FORGE_LOG_WARNING) && defined(DEBUG)
  #define FORGE_LOG_WARNING(MESSAGE, ...) logOutput(LOG_LEVEL_WARNING, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#else
  #define FORGE_LOG_WARNING(MESSAGE, ...)
#endif

#if !defined(FORGE_LOG_INFO) && defined(DEBUG)
  #define FORGE_LOG_INFO(MESSAGE, ...) logOutput(LOG_LEVEL_INFO, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#else
  #define FORGE_LOG_INFO(MESSAGE, ...)
#endif

#if !defined(FORGE_LOG_DEBUG) && defined(DEBUG)
  #define FORGE_LOG_DEBUG(MESSAGE, ...) logOutput(LOG_LEVEL_DEBUG, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#else
  #define FORGE_LOG_DEBUG(MESSAGE, ...)
#endif

#if !defined(FORGE_LOG_TRACE) && defined(DEBUG)
  #define FORGE_LOG_TRACE(MESSAGE, ...) logOutput(LOG_LEVEL_TRACE, MESSAGE __VA_OPT__(,) __VA_ARGS__);
#else
  #define FORGE_LOG_TRACE(MESSAGE, ...)
#endif

#ifndef FORGE_LOG_CLEAR
  #define FORGE_LOG_CLEAR() printf("\033[H\033[J")
#endif

#ifdef __cplusplus
}
#endif
