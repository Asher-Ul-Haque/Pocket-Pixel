#include "../include/testManager.h"
#include "../include/asserts.h"
#include "../include/logger.h"

// - - - Static pointer to the head of the linked list
static TestEntry* testListHead = NULL;

void registerTest(TEST FUNCTION_TO_BE_TESTED, const char* DESCRIPTION) 
{
  // - - - Allocate memory for the new test entry
  TestEntry* newEntry   = (TestEntry*)malloc(sizeof(TestEntry));
  FORGE_ASSERT_MESSAGE(newEntry, "Failed to allocate memory for test entry");

  // - - - Initialize the test entry
  newEntry->func        = FUNCTION_TO_BE_TESTED;
  newEntry->description = strdup(DESCRIPTION); // - - - Duplicate the description string
  newEntry->next        = NULL;

  // - - - Add the test entry to the end of the linked list
  if (!testListHead) 
  {
    testListHead      = newEntry;
  } 
  else 
  {
    TestEntry* current  = testListHead;
    while (current->next) 
    {
      current           = current->next;
    }
    current->next       = newEntry;
  }
}

void runTests() 
{
  u64 passed            = 0;
  u64 failed            = 0;
  u64 skipped           = 0;
  u64 total             = 0;

  TestEntry* current    = testListHead;

  FORGE_LOG_INFO("\n- - - Running Tests - - -\n");
  while (current) 
  {
    total++;
    FORGE_LOG_DEBUG("Testing %llu: %s ... ", total, current->description);

    u8 result           = current->func();
    if (result == true) 
    {
      FORGE_LOG_INFO("Test %llu : PASSED\n", total);
      passed++;
    } 
    else if (result == SKIP_TEST) 
    {
      FORGE_LOG_WARNING("Test %llu : SKIPPED\n", total);
      skipped++;
    } 
    else 
    {
      FORGE_LOG_ERROR("Test %llu : FAILED\n", total);
      failed++;
    }

    current             = current->next;
  }

  FORGE_LOG_INFO("- - - Results - - -");
  FORGE_LOG_DEBUG("Total: \t\t%llu", total);
  FORGE_LOG_INFO("Passed: \t%llu", passed);
  FORGE_LOG_ERROR("Failed: \t%llu", failed);
  FORGE_LOG_WARNING("Skipped: \t%llu", skipped);

  // - - - Free the linked list memory
  current               = testListHead;
  while (current) 
  {
      TestEntry* next   = current->next;
      free(current->description);
      free(current);
      current           = next;
  }

  testListHead          = NULL;
}
