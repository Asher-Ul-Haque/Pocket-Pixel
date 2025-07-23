#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif
#define SKIP_TEST 2

// - - - Test Function
typedef u8 (*TEST)();

// - - - Test 
typedef struct TestEntry 
{
  TEST              func;
  char*             description;
  struct TestEntry* next;
} TestEntry;


// - - - User Interface - - - 

FORGE_API void registerTest           (TEST FUNCTION_TO_BE_TESTED, const char* DESCRIPTION);
FORGE_API void runTests               ();

#ifdef __cplusplus
}
#endif
