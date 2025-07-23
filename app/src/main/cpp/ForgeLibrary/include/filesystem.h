#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include <sys/stat.h>

#ifndef USE_SYSCALLS
  #define USE_SYSCALLS 0
#endif

#if USE_SYSCALLS == 1
  #include <fcntl.h>
  #include <unistd.h>
#endif

typedef         void*                   (memoryAllocate)    (unsigned long SIZE);

typedef enum fileModes
{
  FILE_MODE_READ          = 0x1,
  FILE_MODE_WRITE         = 0x2,
  FILE_MODE_READ_WRITE    = 0x3,
} FileModes;

typedef struct File 
{
  #if USE_SYSCALLS == 1
    int     handle;
  #else 
    FILE*   handle;
  #endif

  bool      isValid;
  FileModes mode;
} File;

FORGE_API unsigned long long  getFileSize         (File* INPUT_FILE);

FORGE_API bool                fileExists          (const char* PATH);

FORGE_API bool                openFile            (const char* PATH,        FileModes MODE,               bool BINARY_MODE, File* OUTPUT_FILE);

FORGE_API void                closeFile           (File* INPUT_FILE);

FORGE_API bool                readFileLine        (File* INPUT_FILE,        char** BUFFER);

FORGE_API bool                readFileLineAlloc   (File* INPUT_FILE,        char** BUFFER,                memoryAllocate* ALLOCATOR);

FORGE_API bool                writeFileLine       (File* INPUT_FILE,        const char* TEXT);

FORGE_API bool                readFile            (File* INPUT_FILE,        unsigned long long DATA_SIZE, void* BUFFER, unsigned long long* READ_BYTES);

FORGE_API bool                readAllBytes        (File* INPUT_FILE,        unsigned char** OUTPUT,       unsigned long long* READ_BYTES);

FORGE_API bool                writeFile           (File* INPUT_FILE,        unsigned long long DATA_SIZE, const void* DATA, unsigned long long* WRITE_BYTES);

FORGE_API bool                setCursor           (File* INPUT_FILE,        unsigned long long CURSOR_POS);

FORGE_API bool                offsetCursor        (File* INPUT_FILE,        long long OFFSET);

FORGE_API unsigned long long  getCursor           (File* INPUT_FILE);

FORGE_API bool                renameFile          (const char* INPUT_PATH,  const char* DESTINATION_PATH);

FORGE_API bool                copyFile            (const char* INPUT_PATH,  const char* DESTINATION_PATH);

FORGE_API bool                deleteFile          (const char* PATH);

FORGE_API bool                compareFile         (File* FILE_1,            File* FILE_2);

FORGE_API bool                compareFileContents (File* FILE_1,            File* FILE_2);

FORGE_API bool                createDirectory     (const char* PATH,        bool RECURSIVE);

FORGE_API bool                deleteDirectory     (const char* PATH,        bool RECURSIVE);

#ifdef __cplusplus
}
#endif
