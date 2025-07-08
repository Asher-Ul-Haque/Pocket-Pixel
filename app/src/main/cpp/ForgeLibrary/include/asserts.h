#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - Assertion enable toggle
#ifndef FORGE_ASSERTS_ENABLED
  #define FORGE_ASSERTS_ENABLED 1
#endif


// - - - | Assertions | - - -


#if FORGE_ASSERTS_ENABLED == 1
  #if _MSC_VER
    #include <intrin.h>
    #define debugBreak() __debugbreak()
  #else
    #define debugBreak() __builtin_trap()
#endif


// - - - Assert Methods - - -

FORGE_API void reportAssertionFailure(const char* EXPRESSION, 
                                      const char* MESSAGE, 
                                      const char* FILE, 
                                      const char* FUNCTION, 
                                      int LINE);

FORGE_API void reportTODO(const char* COMMENT, 
                          const char* FILE, 
                          const char* FUNCTION, 
                          int         LINE);

#define FORGE_ASSERT(EXPRESSION)                                                                  \
{                                                                                                 \
  if (EXPRESSION){}                                                                               \
  else                                                                                            \
  {                                                                                               \
    reportAssertionFailure(#EXPRESSION, "Assert Fail\t", __FILE__, __func__, __LINE__);           \
    debugBreak();                                                                                 \
  }                                                                                               \
}                               

#define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE)                                                 \
{                                                                                                 \
  if (EXPRESSION){}                                                                               \
  else                                                                                            \
  {                                                                                               \
    reportAssertionFailure(#EXPRESSION, MESSAGE, __FILE__, __func__, __LINE__);                   \
    debugBreak();                                                                                 \
  }                                                                                               \
}

#define TODO                    reportTODO(NULL,    __FILE__, __func__, __LINE__);  
#define TODO_COMMENT(COMMENT)   reportTODO(COMMENT, __FILE__, __func__, __LINE__); 

#if LOG_DEBUG_ENABLED == 1
  #define FORGE_ASSERT_DEBUG(EXPRESSION)                                                          \
  {                                                                                               \
    if (EXPRESSION){}                                                                             \
    else                                                                                          \
    {                                                                                             \
    reportAssertionFailure(#EXPRESSION, "Assert Fail\t", __FILE__, __func__, __LINE__);           \
      debugBreak();                                                                               \
    }                                                                                             \
  }
#else 
  #define FORGE_ASSERT_DEBUG(EXPRESSION) // - - - Does nothing at all
#endif

// - - - Assertion Disabled
#else
  #define FORGE_ASSERT(EXPRESSION)                  // - - - Does nothing at all
  #define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE) // - - - Does nothing at all
  #define FORGE_ASSERT_DEBUG(EXPRESSION)            // - - - Does nothing at all
  #define TODO_COMMENT(COMMENT)                     // - - - Does nothing at all
  #define TODO                                      // - - - Does nothing at all
#endif

#ifdef __cplusplus
}
#endif
