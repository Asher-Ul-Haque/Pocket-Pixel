#include "../include/logger.h"
#ifdef __ANDROID__
  #include <android/log.h>
#endif


// - - - Writing Controls - - - 

void writeConsole(const char* MESSAGE, unsigned char COLOR)
{
  // - - - FATAL, ERROR, WARN, INFO, DEBUG, TRACE : damn color codes work on nvim
  const char* colorStrings[] = { "41", "31", "38;5;216", "32", "34", "0"};

#ifdef __ANDROID__
  const char* logTag = "FORGE_LOGGER";
    int androidLogLevel;
    switch (COLOR)
    {
        case 0  : androidLogLevel = ANDROID_LOG_FATAL;    break;
        case 1  : androidLogLevel = ANDROID_LOG_ERROR;    break;
        case 2  : androidLogLevel = ANDROID_LOG_WARN;     break;
        case 3  : androidLogLevel = ANDROID_LOG_INFO;     break;
        case 4  : androidLogLevel = ANDROID_LOG_DEBUG;    break;
        case 5  : androidLogLevel = ANDROID_LOG_VERBOSE;  break;
        default : androidLogLevel = ANDROID_LOG_DEFAULT;  break;
    }
    __android_log_print(androidLogLevel, logTag, "%s", MESSAGE);
#else
  printf("\033[%sm%s\033[0m", colorStrings[COLOR], MESSAGE); // - - -This looks scary
  /*Here is how it works:
  \033[     - This is the escape character
  %s        - This is the color code
  m         - This is the end of the color code
  %s        - This is the message
  \033[0m   - This is the end of the color
  This is how you print colored text in the terminal*/
#endif
}

void writeConsoleError(const char* MESSAGE, unsigned char COLOR) {
  const char *colorStrings[] = {"1;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  fprintf(stderr, "\033[%sm%s\033[0m", colorStrings[COLOR], MESSAGE); // - - - This looks scary

#ifdef __ANDROID__
  const char* logTag = "FORGE_LOGGER";
  int androidLogLevel;
  switch (COLOR)
  {
    case 0  : androidLogLevel = ANDROID_LOG_FATAL;    break;
    case 1  : androidLogLevel = ANDROID_LOG_ERROR;    break;
    case 2  : androidLogLevel = ANDROID_LOG_WARN;     break;
    case 3  : androidLogLevel = ANDROID_LOG_INFO;     break;
    case 4  : androidLogLevel = ANDROID_LOG_DEBUG;    break;
    case 5  : androidLogLevel = ANDROID_LOG_VERBOSE;  break;
    default : androidLogLevel = ANDROID_LOG_DEFAULT;  break;
  }
  __android_log_print(androidLogLevel, logTag, "%s", MESSAGE);
#endif
}


// - - - API Controls - - -

void logOutput(LogLevel LEVEL, const char* MESSAGE, ...)
{
  const char* levelStrings[6]   = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
  const bool  isError           = LEVEL < LOG_LEVEL_WARNING;
  const int   messageLength     = 32000;
  char        outputMessage     [messageLength];
  memset(outputMessage, 0, sizeof(outputMessage));

  // - - - Add the rest of the arguments
  __builtin_va_list argumentPointer;
  va_start(argumentPointer, MESSAGE);
  vsnprintf(outputMessage, sizeof(outputMessage), MESSAGE, argumentPointer);
  va_end(argumentPointer);

  // - - - Prepend with level header
  char finalMessage[32000];
  #if PRINT_LOG_TYPES == 1
    sprintf(finalMessage, "%s\t%s\n", levelStrings[LEVEL], outputMessage);
  #else 
    sprintf(finalMessage, "%s\n", outputMessage);
  #endif

  // - - - write to the console
  if (isError)
  {
    writeConsoleError(finalMessage, LEVEL);
  }
  else 
  {
    writeConsole(finalMessage, LEVEL);    
  }
}


// - - - | Assert Functions | - - -


void reportAssertionFailure(const char* EXPRESSION, const char* MESSAGE, const char* FILE, int LINE)
{
  logOutput(LOG_LEVEL_FATAL, "ASSERT FAIL :     %s\nMESSAGE     :     %s\nLOCATION    :     file: %s at line: %d\n", EXPRESSION, MESSAGE, FILE, LINE);
}

void reportTODO(const char* COMMENT, const char* FILE, const char* FUNCTION, int LINE)
{
    logOutput(LOG_LEVEL_ERROR, "Oopsie!, you have a TODO!!!");
    logOutput(LOG_LEVEL_ERROR, "At: %s:%d -> Function: %s", FILE, LINE, FUNCTION);
    if (COMMENT) logOutput(LOG_LEVEL_WARNING, "COMMENT: %s", COMMENT);
    debugBreak();
}
