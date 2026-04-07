#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <utils/testManager.h>
#include <utils/asserts.h>
#include <utils/logger.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

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

void runTests(void) 
{
  u64 passed            = 0;
  u64 failed            = 0;
  u64 crashed           = 0;
  u64 skipped           = 0;
  u64 total             = 0;

  TestEntry* current    = testListHead;
  char logDir[256];

  FORGE_LOG_INFO("%s", "\n- - - Running Tests - - -\n");
  if (current)
  {
    // - - - create the logs directory
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(logDir, sizeof(logDir), "test_logs_%04d-%02d-%02d_%02d-%02d-%02d", 
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    mkdir(logDir, 0777);
  }

  while (current) 
  {
    total++;
    pid_t pid = fork(); // - - - each test is its own process

    if (pid == -1)
    {
      FORGE_LOG_ERROR("Failed to create a child process for test %llu: %s", total, current->description);
      failed++;
    }
    else if (pid == 0)
    {
      // - - - Child process : Run the test 
      FORGE_LOG_DEBUG("Testing %llu: %s ... ", total, current->description);
      char logFilename[256];
      snprintf(logFilename, sizeof(logFilename), "%s/test_%lu.justForgeLog", logDir, total);
      int fd = open(logFilename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd < 0)
      {
        FORGE_LOG_ERROR("Failed to create log file : %s", logFilename);
        exit(1);
      }

      // - - - redirect stdout and stderr to log file 
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      close(fd);

      exit(current->func()); // - - - exit with the text result
    }
    else 
    {
      // - - - parent waits for the child 
      i32 status;
      waitpid(pid, &status, 0);

      if (WIFEXITED(status))
      {
        u8 result = WEXITSTATUS(status);
        if (result == TEST_RESULT_PASSED) 
        {
          FORGE_LOG_INFO("Test %llu : PASSED\n", total);
          passed++;
        } 
        else if (result == TEST_RESULT_SKIPPED) 
        {
          FORGE_LOG_WARNING("Test %llu : SKIPPED\n", total);
          skipped++;
        } 
        else if (result == TEST_RESULT_FAILED)
        {
          FORGE_LOG_ERROR("Test %llu : FAILED\n", total);
          failed++;
        }
      }
      else if (WIFSIGNALED(status))
      {
        i32 sig = WTERMSIG(status);
        FORGE_LOG_ERROR("Test %llu : CRASHED (Signal %d - %s)\n", total, sig, strsignal(sig));
        crashed++;
      }
    }

    current             = current->next;
  }

  FORGE_LOG_INFO("%s", "- - - Results - - -");
  FORGE_LOG_DEBUG("Total: \t\t%llu", total);
  if (passed  > 0)   FORGE_LOG_INFO("Passed: \t%llu", passed);
  if (skipped > 0)   FORGE_LOG_WARNING("Skipped: \t%llu", skipped);
  if (failed  > 0)   FORGE_LOG_ERROR("Failed: \t%llu", failed);
  if (crashed > 0)   FORGE_LOG_FATAL("Crashed: \t%llu", crashed);

  // - - - ask the user about log file handling-
  FORGE_LOG_DEBUG("%s", "Do you want to (S)ave, or (V)iew logs? (Any other key to delete them)");
  char choice = getchar();
  choice = (char)toupper(choice);
  char command[512];

  if (choice == 'V')
  {
    DIR *dir = opendir(logDir);
    if (!dir)
    {
      FORGE_LOG_ERROR("%s", "Failed to open log directory");
      return;
    }

    struct dirent *entry;
    char filepath[512];
    char buffer[1024];

    while ((entry = readdir(dir)) != NULL)
    {
      if (entry->d_name[0] == '.') continue;

      snprintf(filepath, sizeof(filepath), "%s/%s", logDir, entry->d_name);

      FILE *f = fopen(filepath, "r");
      if (!f) continue;

      printf("\n==== %s ====\n", entry->d_name);

      while (fgets(buffer, sizeof(buffer), f))
      {
        fputs(buffer, stdout);
      }

      fclose(f);
    }

    closedir(dir);
  }

  if (choice == 'S')
  {
    FORGE_LOG_INFO("Logs saved in the directory : %s", logDir);
  }
  else 
  {
    snprintf(command, sizeof(command), "rm -rf %s", logDir);
    system(command);
    FORGE_LOG_WARNING("%s", "Logs deleted.");
  }

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
