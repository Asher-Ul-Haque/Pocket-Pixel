/**
 * @file testManager.h
 * @brief A simple test manager for registering and running tests.
 * It also includes plenty of expect macros for making assertions in your tests.
 * @note Taken from Forge Library, 
 * @see https://github.com/Asher-Ul-Haque/ForgeLibrary
*/

#pragma once
#include <defines.h>
#include <utils/dataTypes.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The TestResult enum defines the possible outcomes of a test, where:
 * - TEST_RESULT_PASSED (1) indicates that the test passed successfully.
 * - TEST_RESULT_FAILED (0) indicates that the test failed.
 * - TEST_RESULT_SKIPPED (2) indicates that the test was skipped, which can be useful for tests that are not applicable in certain environments or conditions.
 * - Any other values means that the test crashed 
*/
typedef enum TestResult 
{
  TEST_RESULT_FAILED  = 0, 
  TEST_RESULT_PASSED  = 1, 
  TEST_RESULT_SKIPPED = 2  
} TestResult;


/**
 * @brief The TEST type defines a function pointer type for test functions, which should return a TestResult indicating the outcome of the test.
 * Each test function should return TEST_RESULT_PASSED if the test passes, 
 * TEST_RESULT_FAILED if the test fails, 
 * or TEST_RESULT_SKIPPED if the test is skipped.
*/
typedef TestResult (*TEST)(void);

/**
 * @brief The TestEntry struct represents an individual test entry in the test manager, containing:
 * - func: a function pointer to the test function that will be executed.
 * - description: a string describing the test, which can be used for reporting and logging purposes
 * - next: a pointer to the next TestEntry in the linked list, allowing for multiple tests to be registered and executed in sequence.  
 * This structure allows the test manager to maintain a linked list of registered tests, enabling dynamic addition of tests and sequential execution when running the tests.
*/
typedef struct TestEntry 
{
  TEST              func;
  char*             description;
  struct TestEntry* next;
} TestEntry;


// - - - User Interface - - - 

/**
 * @brief Registers a test function with the test manager, allowing it to be executed when runTests is called. 
 * The DESCRIPTION parameter provides a human-readable description of the test, which can be used for reporting and logging purposes.
 * @param FUNCTION_TO_BE_TESTED A function pointer to the test function that will be executed. This function should return a TestResult indicating the outcome of the test.
 * @param DESCRIPTION A string describing the test, which can be used for reporting and logging purposes.
*/
void registerTest (TEST FUNCTION_TO_BE_TESTED, const char* DESCRIPTION);

/**
 * @brief Executes all registered tests in the test manager, running each test function and reporting the results. 
 * The test manager will iterate through the linked list of registered tests, execute each test function, and log the outcome (passed, failed, skipped) along with the test description. 
 * This function serves as the main entry point for running all tests that have been registered with the test manager.
 * It prints a summary of the test results, including the number of tests passed, failed, skipped, and any tests that may have crashed during execution. 
*/
void runTests     (void);


// - - - | Expect Macros | - - - 

/// @brief Verifies two values are equal.
#define EXPECT_TO_BE(EXPECTED, ACTUAL)                                                                    \
if ((ACTUAL) != (EXPECTED))                                                                               \
{                                                                                                         \
  FORGE_LOG_ERROR("--> EXPECTED %s, but got: %s. File: %s:%d.", #EXPECTED, #ACTUAL, __FILE__, __LINE__);  \
  return TEST_RESULT_FAILED;                                                                              \
}

/// @brief Verifies two values are not equal.
#define EXPECT_NOT_TO_BE(NOT_EXPECTED, ACTUAL)                                                                    \
if ((ACTUAL) == (NOT_EXPECTED))                                                                                       \
{                                                                                                                 \
  FORGE_LOG_ERROR("--> NOT_EXPECTED %s, but got: %s. File: %s:%d.", #NOT_EXPECTED, #ACTUAL, __FILE__, __LINE__);  \
  return TEST_RESULT_FAILED;                                                                                      \
}

/// @brief Verifies two string values are equal.
#define EXPECT_STRING_TO_BE(EXPECTED, ACTUAL)                                                         \
if (strcmp(EXPECTED, ACTUAL) != 0)                                                                    \
{                                                                                                     \
  FORGE_LOG_ERROR("--> EXPECTED %s, but got: %s. File: %s:%d", EXPECTED, ACTUAL, __FILE__, __LINE__)  \
  return TEST_RESULT_FAILED;                                                                          \
}

/**
 * @brief Verifies two floating point values are approximately equal.
 * @param EPS Allowed tolerance.
*/
#define EXPECT_FLOAT_TO_BE(EXPECTED, ACTUAL, EPS)                                                          \
if (((EXPECTED) - (ACTUAL) < EPS) || ((EXPECTED) - (ACTUAL) > EPS))                                        \
{                                                                                                          \
  FORGE_LOG_ERROR("--> EXPECTED %f, but got: %f. File: %s:%d.", (EXPECTED), (ACTUAL), __FILE__, __LINE__); \
  return TEST_RESULT_FAILED;                                                                               \
}

/// @brief Verifies a pointer is null.
#define EXPECT_TO_BE_NULL(PTR)                                \
if ((PTR) != NULL)                                            \
{                                                             \
  FORGE_LOG_ERROR("--> EXPECT_TO_BE_NULL failed : %s", #PTR); \
  return TEST_RESULT_FAILED;                                  \
}

/// @brief Verifies a pointer is not null.
#define EXPECT_TO_BE_NOT_NULL(PTR)                                \
if ((PTR) == NULL)                                                \
{                                                                 \
  FORGE_LOG_ERROR("--> EXPECT_TO_BE_NOT_NULL failed : %s", #PTR); \
  return TEST_RESULT_FAILED;                                      \
}

/// @brief Verifies an expression evaluates to true.
#define EXPECT_TO_BE_TRUE(EXPR)                                 \
if (!(EXPR))                                                    \
{                                                               \
  FORGE_LOG_ERROR("--> EXPECT_TO_BE_TRUE failed : %s", #EXPR);  \
  return TEST_RESULT_FAILED;                                    \
}

/// @brief Verifies an expression evaluates to false.
#define EXPECT_TO_BE_FALSE(EXPR)                          \
if ((EXPR))                                               \
{                                                         \
  LOG_ERROR("--> EXPECT_TO_BE_FALSE failed : %s", #EXPR); \
  return TEST_RESULT_FAILED;                              \
}

#ifdef __cplusplus
}
#endif
