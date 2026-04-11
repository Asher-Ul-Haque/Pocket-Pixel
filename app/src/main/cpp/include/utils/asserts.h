/**
 * @file asserts.h
 * @brief Assertion and TODO helper macros
 *
 * Provides compile-time assertions, runtime assertions and TODO 
 * markers that log an error before terminating the program.
 *
 * Assertions can be enabled or disabled globally using the FORGE_ASSERTS_ENABLED macro.
 * When enabled, failed assertions will log the expression, an optional message,
 * and the file, function, and line number where the assertion failed. The program will then break into the debugger.
 * When disabled, all assertion macros will do nothing and have no runtime cost.
 * @note Taken from Forge Library, 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
*/

#pragma once
#include <utils/logger.h>
#include <utils/dataTypes.h>
#include <defines.h>

#ifdef __cplusplus
extern "C" {
#endif

// - - - For debuggers 
#if FORGE_ASSERTS_ENABLED == 1
  #if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
  #else
    #define debugBreak() __builtin_trap()
  #endif
#endif


// - - - | Assertions | - - -

#if defined(__clang__) || defined(__gcc__)
  #ifdef __cplusplus 
    #define COMPILE_TIME_ASSERT static_assert
  #else   
    #define COMPILE_TIME_ASSERT _Static_assert
  #endif
#else
  #define COMPILE_TIME_ASSERT static_assert
#endif

// - - - Assert Methods - - -

/**
 * @brief Reports a failed assertion with detailed information about the failure.
 * @param EXPRESSION The expression that was evaluated and failed (as a string).
 * @param MESSAGE An optional custom message to provide additional context about the failure.
 * @param FILE The name of the source file where the assertion failed.
 * @param FUNCTION The name of the function where the assertion failed.
 * @param LINE The line number in the source file where the assertion failed.
 *
 * This function is called when an assertion fails. It logs the details of the failure, including the expression that failed, an optional message, and the location in the code where it occurred. After logging, it is expected that the caller will break into the debugger or terminate the program.
 * @warning This function is intended to be called by the assertion macros (e.g., FORGE_ASSERT) and should not be called directly in most cases. It is designed to provide detailed information about assertion failures to aid in debugging.
 * @see FORGE_ASSERT, FORGE_ASSERT_MESSAGE and FORGE_ASSERT_DEBUG for examples of how this function is used in the assertion macros.
*/
void reportAssertionFailure(const char* EXPRESSION, 
                            const char* MESSAGE, 
                            const char* FILE, 
                            const char* FUNCTION, 
                            i32         LINE);

/**
 * @brief Reports a TODO marker with an optional comment and location information.
 * @param COMMENT An optional comment describing the TODO item.
 * @param FILE The name of the source file where the TODO is located.
 * @param FUNCTION The name of the function where the TODO is located.
 * @param LINE The line number in the source file where the TODO is located.
 * This function is called to report a TODO item in the code. 
 * It logs the provided comment (if any) along with the location in the code where the TODO is placed. 
 * This can be used to track unfinished work or areas that need attention without causing a program failure.
 * @warning This function is intended to be called by the TODO macros (e.g., TODO and TODO_COMMENT) and should not be called directly in most cases. It is designed to provide information about TODO items in the code to aid in tracking and development.
 * @see TODO and TODO_COMMENT macros for examples of how this function is used to report TODO items in the code.
*/
void reportTODO(const char* COMMENT, 
                const char* FILE, 
                const char* FUNCTION, 
                i32         LINE);

#if FORGE_ASSERTS_ENABLED == 1
  /**
   * @brief Evaluates an expression and reports a failed assertion if the expression is false.
   * @param EXPRESSION The expression to evaluate. If the expression evaluates to false,
   * the assertion is considered failed, and the failure details are reported.
   * This macro checks the provided expression. If the expression evaluates to true, it does nothing.
   * If the expression evaluates to false, it calls the reportAssertionFailure function with the expression
   * as a string, a default message "Assert Fail\t", and the file, function, and line information where the assertion failed. 
   * After reporting the failure, it breaks into the debugger.
   * @note This macro is only active when FORGE_ASSERTS_ENABLED is set to 1. If assertions are disabled, this macro does nothing and has no runtime cost.
   * @warning Be cautious when using this macro in performance-critical code, as it will evaluate the expression and potentially log information on failure. 
   * Consider using FORGE_ASSERT_DEBUG for assertions that should only be active in debug builds.
   * @see FORGE_ASSERT_DEBUG for assertions that are only active in debug builds.
  */
  #define FORGE_ASSERT(EXPRESSION)                                                                  \
  {                                                                                                 \
    if (EXPRESSION){}                                                                               \
    else                                                                                            \
    {                                                                                               \
      reportAssertionFailure(#EXPRESSION, "Assert Fail\t", __FILE__, __func__, __LINE__);           \
      debugBreak();                                                                                 \
    }                                                                                               \
  }                               

  /**
   * @brief Evaluates an expression and reports a failed assertion with a custom message if the expression is false.
   * @param EXPRESSION The expression to evaluate. If the expression evaluates to false,
   * the assertion is considered failed, and the failure details are reported along with a custom message
   * @param MESSAGE A custom message to provide additional context about the assertion failure. This message will be included in the log output when the assertion fails.
   * This macro checks the provided expression. If the expression evaluates to true, it does nothing
   * If the expression evaluates to false, it calls the reportAssertionFailure function with the expression as a string, the provided custom message, and the file, function, and line information where the assertion failed.
   * After reporting the failure, it breaks into the debugger.
   * @note This macro is only active when FORGE_ASSERTS_ENABLED is set to 1. 
   * If assertions are disabled, this macro does nothing and has no runtime cost.
   * @warning Be cautious when using this macro in performance-critical code, as it will evaluate the expression and potentially log information on failure. 
   * Consider using FORGE_ASSERT_DEBUG for assertions that should only be active in debug builds.
   * @see FORGE_ASSERT_DEBUG for assertions that are only active in debug builds.
  */
  #define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE)                                                 \
  {                                                                                                 \
    if (EXPRESSION){}                                                                               \
    else                                                                                            \
    {                                                                                               \
      reportAssertionFailure(#EXPRESSION, MESSAGE, __FILE__, __func__, __LINE__);                   \
      debugBreak();                                                                                 \
    }                                                                                               \
  }

  #ifdef DEBUG
    /** 
    * @brief Evaluates an expression and reports a failed assertion if the expression is false, but only in debug builds.
    * @param EXPRESSION The expression to evaluate. If the expression evaluates to false,
    * the assertion is considered failed, and the failure details are reported, but only when compiled
    * in debug mode (when the DEBUG macro is defined). In release builds, this macro does nothing and has no runtime cost.
    * This macro checks the provided expression. If the expression evaluates to true, it does nothing
    * If the expression evaluates to false, it calls the reportAssertionFailure function with the expression as a string, a default message "Assert Fail\t", and the file, function, and line information where the assertion failed. 
    * After reporting the failure, it breaks into the debugger.
    * @note This macro is only active when the DEBUG macro is defined. If the DEBUG macro is not defined, this macro does nothing and has no runtime cost.
    * @warning If you want assertions that are active in both debug and release builds, use FORGE_ASSERT or FORGE_ASSERT_MESSAGE instead.
    * @see FORGE_ASSERT and FORGE_ASSERT_MESSAGE for assertions that are active in both debug and release builds.
    */
    #define FORGE_ASSERT_DEBUG(EXPRESSION, MESSAGE)                                                   \
    {                                                                                                 \
      if (EXPRESSION){}                                                                               \
      else                                                                                            \
      {                                                                                               \
        reportAssertionFailure(#EXPRESSION, MESSAGE, __FILE__, __func__, __LINE__);                   \
        debugBreak();                                                                                 \
      }                                                                                               \
    }
  
    /**
     * @brief Reports a TODO marker with an optional comment, but only in debug builds.
     * This macro calls the reportTODO function with the file, function, and line information where the TODO is placed.
     * This is intended to be used as a marker for unfinished work or areas that need attention during development.
     * It is only active when the DEBUG macro is defined. If the DEBUG macro is not defined, this macro does nothing and has no runtime cost.
     * @note This macro is only active when the DEBUG macro is defined. If the DEBUG macro is not defined, this macro does nothing and has no runtime cost.
     * @warning Using this macro may allow some functions to be left unimplemented without causing a compile-time error, so use it judiciously to avoid leaving important work unfinished.
    */
    #define TODO                                          \
    {                                                     \
      reportTODO(NULL,    __FILE__, __func__, __LINE__);  \
      debugBreak();                                       \
    }
    
    /**
     * @brief Reports a TODO marker with an optional comment, but only in debug builds.
     * @param COMMENT An optional comment describing the TODO item. This comment will be included in the log output when the TODO is reported. 
     * If no comment is provided, it will be logged as "TODO\t".
     * This macro calls the reportTODO function with the file, function, and line information where the TODO is placed.
     * This is intended to be used as a marker for unfinished work or areas that need attention during development.
     * It is only active when the DEBUG macro is defined. If the DEBUG macro is not defined, this macro does nothing and has no runtime cost.
     * @note This macro is only active when the DEBUG macro is defined. If the DEBUG macro is not defined, this macro does nothing and has no runtime cost.
     * @warning Using this macro may allow some functions to be left unimplemented without causing a compile-time error, so use it judiciously to avoid leaving important work unfinished.
    */
    #define TODO_COMMENT(COMMENT)                         \
    {                                                     \
      reportTODO(COMMENT, __FILE__, __func__, __LINE__);  \
      debugBreak();                                       \
    }
  #else 
    #define FORGE_ASSERT_DEBUG(EXPRESSION, MESSAGE) // - - - Does nothing at all
    #define TODO                                    // - - - Does nothing at all
    #define TODO_COMMENT(COMMENT)                   // - - - Does nothing at all
  #endif


// - - - Assertion Disabled
#else
  #define FORGE_ASSERT(EXPRESSION)                  // - - - Does nothing at all
  #define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE) // - - - Does nothing at all
  #define FORGE_ASSERT_DEBUG(EXPRESSION, MESSAGE)   // - - - Does nothing at all
  #define TODO_COMMENT(COMMENT)                     // - - - Does nothing at all
  #define TODO                                      // - - - Does nothing at all
#endif

#ifdef __cplusplus
}
#endif
