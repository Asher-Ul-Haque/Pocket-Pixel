#include "../include/filesystem.h"
#include "../include/asserts.h"
#include "../include/logger.h"

#ifndef ANDROID
unsigned long long getFileSize(File* INPUT_FILE)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE  != NULL, "INPUT_FILE cannot be null");
  FORGE_ASSERT_MESSAGE(INPUT_FILE->isValid, "INPUT_FILE must be valid");
  
  #if USE_SYSCALLS == 0
  
  fseek(INPUT_FILE->handle, 0, SEEK_END);
  unsigned long long size = ftell(INPUT_FILE->handle);
  rewind(INPUT_FILE->handle);
  return size;

  #else 

  struct stat buffer;
  if (fstat(INPUT_FILE->handle, &buffer) < 0)
  {
    FORGE_LOG_ERROR("Getting negative filesize, maybe file doesnt exist?");
    return -1;
  }
  return buffer.st_size;

  #endif 
}

bool fileExists(const char* PATH)
{
  FORGE_ASSERT_MESSAGE(PATH != NULL, "A filepath cannot be null");
  struct stat buffer;
  return stat(PATH, &buffer) == 0;
}

bool openFile(const char* PATH, FileModes MODE, bool BINARY_MODE, File* OUTPUT_FILE)
{
  FORGE_ASSERT_MESSAGE(PATH           != NULL, "A filepath cannot be null");
  FORGE_ASSERT_MESSAGE(OUTPUT_FILE    != NULL, "You must use a File(struct) to open a file");

  OUTPUT_FILE->handle   =   0;
  OUTPUT_FILE->isValid  =   false;
  bool exists           =   fileExists(PATH);

  if (!exists)
  {
    switch (MODE)
    {
      case FILE_MODE_READ:
        FORGE_LOG_ERROR("The file at : %s does not exist", PATH);
        return false;

      default:
        FORGE_LOG_WARNING("The file at %s does not exist, creating a new one", PATH);
    }
  }
  
  #if USE_SYSCALLS      ==  0

    const char* modeStr;
    if ((MODE & FILE_MODE_READ) != 0 && (MODE & FILE_MODE_WRITE) != 0)
    {
      modeStr = BINARY_MODE ? "wb+" : "w+";
    }
    else if ((MODE & FILE_MODE_READ) != 0 && (MODE & FILE_MODE_WRITE) == 0)
    {
      modeStr = BINARY_MODE ? "rb" : "r";
    }
    else if ((MODE & FILE_MODE_WRITE) != 0 && (MODE & FILE_MODE_READ) == 0)
    {
      modeStr = BINARY_MODE ? "wb" : "w";
    }
    else
    {
      FORGE_LOG_ERROR("Invalid file mode (%d) passeed while trying to open file: %s", MODE, PATH);
      return false;
    }

    // - - - Attempt to open file
    FILE* file = fopen(PATH, modeStr);
    if (!file)
    {
      FORGE_LOG_ERROR("Failed to open file: %s", PATH);
      return false;
    }

    OUTPUT_FILE->handle   = file;
    OUTPUT_FILE->isValid  = true;
    OUTPUT_FILE->mode     = MODE;

  #else

    FORGE_LOG_TRACE("Using open syscall to open a file");
    int flags;
    if ((MODE & FILE_MODE_READ) && (MODE & FILE_MODE_WRITE)) 
    {
      flags = O_RDWR | O_CREAT;
    } else if (MODE & FILE_MODE_READ) 
    {
      flags = O_RDONLY;
    } else if (MODE & FILE_MODE_WRITE) 
    {
      flags = O_WRONLY | O_CREAT;
    }
  
    int fd = open(PATH, flags, 0644);
    if (fd < 0) 
    {
      FORGE_LOG_ERROR("Failed to open file: %s", PATH);
      return false;
    }

    OUTPUT_FILE->handle   = fd;
    OUTPUT_FILE->isValid  = true;
    OUTPUT_FILE->mode     = MODE;

  #endif

  return true;
}

void closeFile(File* INPUT_FILE)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE != NULL, "Cannot close a null input file");

  if (!INPUT_FILE->isValid) goto warn;

  #if USE_SYSCALLS          ==    0

    fclose((FILE*)INPUT_FILE->handle);

  #else 
  
    FORGE_LOG_TRACE("Using the close syscall");
    close(INPUT_FILE->handle);
    
  #endif

  INPUT_FILE->handle        =   0;
  INPUT_FILE->isValid       =   false;
  return;
  
  warn:
    FORGE_LOG_WARNING("Attempted to close an invalid file");
}

bool readFileLine(File* INPUT_FILE, char** BUFFER)
{
  return readFileLineAlloc(INPUT_FILE, BUFFER, malloc);
}

bool readFileLineAlloc(File* INPUT_FILE, char** BUFFER, memoryAllocate ALLOCATOR)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE                           != NULL, "INPUT_FILE cannot be null for reading");
  FORGE_ASSERT_MESSAGE((INPUT_FILE->mode & FILE_MODE_READ)  == 1, "INPUT_FILE must have read permissions");
  FORGE_ASSERT_MESSAGE(BUFFER                               != NULL, "BUFFER cannot be null for reading");

  if (!INPUT_FILE->handle) goto warn;
  
  // - - - NOTE: Since we are reading a single line, assume to assume that 32000 is enough characters
  char buffer[32000];

  #if USE_SYSCALLS == 0
    
    if (fgets(buffer, 32000, (FILE*)INPUT_FILE->handle) != 0)
    {
      unsigned long long length = strlen(buffer);
      *BUFFER = ALLOCATOR((sizeof(char) * length) + 1);
      FORGE_ASSERT_MESSAGE(BUFFER != NULL, "Failed to malloc for the buffer. Maybe file line too big");
      strcpy(*BUFFER, buffer);
      return true;
    }
    goto warn;

  #else

    FORGE_LOG_TRACE("Using read syscall");

    unsigned long long position = 0;
    char ch;
    
  ssize_t bytesRead = 0;
  while ((bytesRead = read(INPUT_FILE->handle, &ch, 1)) == true)
  {
    if (ch == '\n' || ch == '\0') // - - - Stop at newline or null terminator
    {
      buffer[position] = '\0'; // - - - Null-terminate the string
      unsigned long long length = strlen(buffer);
      *BUFFER = ALLOCATOR((sizeof(char) * length) + 1);
      FORGE_ASSERT_MESSAGE(BUFFER != NULL, "Failed to malloc for the buffer. Maybe file line too big");
      strcpy(*BUFFER, buffer);
      return true;
    }

    if (position < sizeof(buffer) - 1)
    {
      buffer[position++] = ch; // - - - Append character to the buffer
    }
    else
    {
      FORGE_LOG_WARNING("Line exceeds maximum buffer size of 32000 characters");
      break;
    }
  }


  #endif 

  warn:
    FORGE_LOG_WARNING("Attempted to read from an invalid file. Maybe the file is empty?");
    return false;
}

bool writeFileLine(File* INPUT_FILE, const char* TEXT)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE                           != NULL, "INPUT_FILE cannot be null for writing");
  FORGE_ASSERT_MESSAGE((INPUT_FILE->mode & FILE_MODE_WRITE) == 2,    "INPUT_FILE must have write permissions");
  FORGE_ASSERT_MESSAGE(TEXT                                 != NULL, "TEXT cannot be null for writing");

  if (!INPUT_FILE->handle) goto warn;
  #if USE_SYSCALLS == 0

    int result = fputs(TEXT, (FILE*)INPUT_FILE->handle);
    if (result != EOF)
    {
      result = fputc('\n', (FILE*)INPUT_FILE->handle);
    }

    return result != EOF;

  #else

    FORGE_LOG_TRACE("Using write syscall")
    size_t textLength = strlen(TEXT);
    ssize_t bytesWritten = write(INPUT_FILE->handle, TEXT, textLength);
    if (bytesWritten != textLength)
    {
      FORGE_LOG_ERROR("Failed to write the full text to the file: %s", *TEXT);
      return false;
    }

    // - - - Write the newline character
    char newline = '\n';
    bytesWritten = write(INPUT_FILE->handle, &newline, 1);
    if (bytesWritten != 1)
    {
      FORGE_LOG_ERROR("Failed to write the new line character to the file: %s", *TEXT);
      return false;
    }

    return true;

  #endif

  warn:
    FORGE_LOG_WARNING("Attempted to write to an invalid file");
    return false;
}

bool readFile(File* INPUT_FILE, unsigned long long DATA_SIZE, void* BUFFER, unsigned long long* READ_BYTES)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE                           != NULL, "Cannot read a null input file");
  FORGE_ASSERT_MESSAGE((INPUT_FILE->mode & FILE_MODE_READ)  == 1, "INPUT_FILE must have read permissions");
  FORGE_ASSERT_MESSAGE(BUFFER                               != NULL, "Cannot read to a null buffer");
  FORGE_ASSERT_MESSAGE(READ_BYTES                           != NULL, "Cannot read null bytes");

  #if USE_SYSCALLS == 0

    if (INPUT_FILE->handle)
    {
      *READ_BYTES = fread(BUFFER, 1, DATA_SIZE, (FILE*)INPUT_FILE->handle);
      if (*READ_BYTES != DATA_SIZE)
      {
        FORGE_LOG_WARNING("Failed to read %llu bytes from file", DATA_SIZE);
        return false;
      }
      return true;
    }
    goto warn;

  #else 
    
    FORGE_LOG_TRACE("Using read sycall");
    if (INPUT_FILE->handle)
    {
      ssize_t result = read(INPUT_FILE->handle, BUFFER, DATA_SIZE);
      *READ_BYTES = (unsigned long long) result;
      if (*READ_BYTES != DATA_SIZE)
      {
        FORGE_LOG_WARNING("Failed to read %llu bytes from file", DATA_SIZE);
        return false;
      }
      return true;
    }
    goto warn;

  #endif

  warn:
    FORGE_LOG_WARNING("Attempted to read from an invalid file. Maybe the file is empty?");
    return false;
}

bool readAllBytes(File* INPUT_FILE, unsigned char** OUTPUT, unsigned long long* READ_BYTES)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE                           != NULL, "Input File cannot be null");
  FORGE_ASSERT_MESSAGE((INPUT_FILE->mode & FILE_MODE_READ)  == 1,    "INPUT_FILE must have read permissions");
  FORGE_ASSERT_MESSAGE(OUTPUT                               != NULL, "OUTPUT String Array cannot be null");
  FORGE_ASSERT_MESSAGE(READ_BYTES                           != NULL, "READ_BYTES cannot be null");

  if (!INPUT_FILE->handle) goto warn;

  // - - - create an output buffer the same size as the file.
  unsigned long long size     = getFileSize(INPUT_FILE);
  unsigned long long position = getCursor(INPUT_FILE);
  *OUTPUT                     = malloc(size);
  FORGE_ASSERT_MESSAGE(OUTPUT != NULL, "Malloc fialed while creating buffer");

  #if USE_SYSCALLS == 0

    rewind((FILE*)INPUT_FILE->handle);
    *READ_BYTES = fread(*OUTPUT, 1, size, (FILE*)INPUT_FILE->handle);
    goto checkSuccess;


  #else
  
    FORGE_LOG_TRACE("Using read syscall");
    *READ_BYTES = read(INPUT_FILE->handle, *OUTPUT, size);
    goto checkSuccess;

  #endif

  checkSuccess:
    if (*READ_BYTES != size - position)
    {
      FORGE_LOG_WARNING("Failed to read the file (%llu bytes), read only (%llu bytes), freeing the buffer", size, *READ_BYTES);
      free(*OUTPUT);
      return false;
    }
    return true;

  warn:
    FORGE_LOG_WARNING("Attempted to read from an invalid file, maybe the file is empty");
    return false;
}

bool writeFile(File* INPUT_FILE, unsigned long long DATA_SIZE, const void* DATA, unsigned long long* WRITE_BYTES)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE                           != NULL, "INPUT_FILE cannot be null for writing");
  FORGE_ASSERT_MESSAGE((INPUT_FILE->mode & FILE_MODE_WRITE) == 2,    "INPUT_FILE must have write permissions");
  FORGE_ASSERT_MESSAGE(DATA                                 != NULL, "BUFFER cannot be null for reading");
  FORGE_ASSERT_MESSAGE(WRITE_BYTES                          != NULL, "WRITE_BYTES cannot be null");

  if (!INPUT_FILE->handle) goto warn;
  #if USE_SYSCALLS == 0

  *WRITE_BYTES = fwrite(DATA, 1, DATA_SIZE, (FILE*)INPUT_FILE->handle);
  if (*WRITE_BYTES != DATA_SIZE)
  {
    FORGE_LOG_WARNING("Failed to write %llu bytes to file", DATA_SIZE);
    return false;
  }
  return true;

  #else
    
    FORGE_LOG_TRACE("Using the write syscall");
    *WRITE_BYTES = write(INPUT_FILE->handle, DATA, DATA_SIZE);
    if (*WRITE_BYTES != DATA_SIZE)
    {
      FORGE_LOG_WARNING("Failed to write %llu bytes to file", DATA_SIZE);
      return false;
    }

    return true;

  #endif

  warn:
    FORGE_LOG_WARNING("Attempted to write to an invalid file");
    return false;
}

bool seekCursor(File* INPUT_FILE, long long OFF_T, int SEEK_HEAD)
{
  if (!INPUT_FILE->isValid) goto warn;

  unsigned long long seekResult;

  #if USE_SYSCALLS == 0
    
    seekResult = fseek(INPUT_FILE->handle, OFF_T, SEEK_HEAD);
    if (seekResult != 0) goto warn;

  #else 

    FORGE_LOG_TRACE("Using lseek syscall to set the cursor position");
    seekResult = lseek(INPUT_FILE->handle, OFF_T, SEEK_HEAD);
    if (seekResult == -1) goto warn;
  
  #endif

  return true;

  warn:
    FORGE_LOG_ERROR("failed to set cursor position. Maybe file is not valid or position if unreasonable");
    return false;
}

bool setCursor(File* INPUT_FILE, unsigned long long CURSOR_POS)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE != NULL, "INPUT_FILE cannot be null for setting the cursor")
  FORGE_ASSERT_MESSAGE(CURSOR_POS >= 0   , "CURSOR_POS must be greater than or equal to 0 to set the cursor position");

  return seekCursor(INPUT_FILE, CURSOR_POS, SEEK_SET);
}

bool offsetCursor(File* INPUT_FILE, long long OFFSET)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE != NULL,                    "INPUT_FILE cannot be null for setting the cursor")
  FORGE_ASSERT_MESSAGE((OFFSET + getCursor(INPUT_FILE)) >= 0, "OFFSET + cursor must be greater than or equal to 0 to set the cursor position");

  return seekCursor(INPUT_FILE, OFFSET, SEEK_CUR);
}

unsigned long long getCursor(File* INPUT_FILE)
{
  FORGE_ASSERT_MESSAGE(INPUT_FILE != NULL, "INPUT_FILE cannot be null for getting the cursor");

  #if USE_SYSCALLS == 0
    
    return ftell(INPUT_FILE->handle);

  #else 
    
    FORGE_LOG_TRACE("Using lseek syscall to get the cursor position");
    return lseek(INPUT_FILE->handle, 0, SEEK_CUR);

  #endif
}

bool renameFile(const char* INPUT_PATH, const char* DESTINATION_PATH)
{
  FORGE_ASSERT_MESSAGE(INPUT_PATH       != NULL, "INPUT_PATH cannot be null");
  FORGE_ASSERT_MESSAGE(DESTINATION_PATH != NULL, "DESTINATION_PATH cannot be null");

  if (!fileExists(INPUT_PATH))
  {
    FORGE_LOG_WARNING("The file at %s does not exist", INPUT_PATH);
    return false;
  }

  return rename(INPUT_PATH, DESTINATION_PATH);
}

bool copyFile(const char* INPUT_PATH, const char* DESTINATION_PATH)
{
  FORGE_ASSERT_MESSAGE(INPUT_PATH       != NULL, "INPUT_PATH cannot be null");
  FORGE_ASSERT_MESSAGE(DESTINATION_PATH != NULL, "DESTINATION_PATH cannot be null");

  if (!fileExists(INPUT_PATH))
  {
    FORGE_LOG_WARNING("The input file at %s does not exist", INPUT_PATH);
    return false;
  }

  if (fileExists(DESTINATION_PATH))
  {
    FORGE_LOG_WARNING("The output file at %s already exists. It will be overwriten", INPUT_PATH);
  }
  
  File input;
  File output;

  if (!openFile(INPUT_PATH, FILE_MODE_READ, true, &input))
  {
    FORGE_LOG_ERROR("Error in opening input file at %s in READ_MODE", INPUT_PATH);
    return false;
  }

  if (!openFile(DESTINATION_PATH, FILE_MODE_WRITE, true, &output))
  {
    FORGE_LOG_ERROR("Error in opening output file at %s in WRITE_MODE", INPUT_PATH);
    return false;
  }

  unsigned char* buffer;
  unsigned long long readBytes = 0;
  unsigned long long writeBytes = 0;
  if (!readAllBytes(&input, &buffer, &readBytes))
  {
    FORGE_LOG_ERROR("Failed to read all the bytes of the input file %s", INPUT_PATH);
    free(buffer);
    return false;
  };

  if (!writeFile(&output, getFileSize(&input), buffer, &writeBytes))
  {
    FORGE_LOG_ERROR("Failed to write all bytes of the output file %s", DESTINATION_PATH);
    free(buffer);
    return false;
  }

  free(buffer);
  closeFile(&output);
  closeFile(&input);

  return true;
}

bool deleteFile(const char* PATH)
{
  if (!fileExists(PATH))
  {
    FORGE_LOG_ERROR("The file at %s does not exist. Cannot delete", PATH);
    return false;
  }

  return remove(PATH);
}

bool compareFile(File* FILE_1, File* FILE_2) 
{
  return (FILE_1 && FILE_2) && (FILE_1->handle == FILE_2->handle) && (FILE_1->mode == FILE_2->mode);
}

bool compareFileContents(File* FILE_1, File* FILE_2)
{
  FORGE_ASSERT_MESSAGE(FILE_1                           != NULL, "FILE_1 cannot be NULL for content comparison");
  FORGE_ASSERT_MESSAGE(FILE_2                           != NULL, "FILE_2 cannot be NULL for content comparison");
  FORGE_ASSERT_MESSAGE((FILE_1->mode & FILE_MODE_READ)  == 1,    "FILE_1 must have read permissions");
  FORGE_ASSERT_MESSAGE((FILE_2->mode & FILE_MODE_READ)  == 1,    "FILE_2 must have read permissions");

  if (!FILE_1->isValid)
  {
    FORGE_LOG_ERROR("FILE_1 is not valid");
    return false;
  }

  if (!FILE_2->isValid)
  {
    FORGE_LOG_ERROR("FILE_2 is not valid");
    return false;
  }

  if (getFileSize(FILE_1) != getFileSize(FILE_2))
  {
    return false;
  }

  unsigned char* buffer1;
  unsigned char* buffer2;
  unsigned long long readBytes1 = 0;
  unsigned long long readBytes2 = 0;

  if (!readAllBytes(FILE_1, &buffer1, &readBytes1))
  {
    goto freeFailBuffer1;
  };
  
  if (!readAllBytes(FILE_2, &buffer1, &readBytes1))
  {
    goto freeFailBuffer2;
  };

  int ret = memcmp(buffer1, buffer2, readBytes1);
  free(buffer1);
  free(buffer2);

  return ret;
  
  freeFailBuffer2:
    FORGE_LOG_ERROR("Failed to read all the bytes of FILE_1");
    free(buffer2);

  freeFailBuffer1:
    FORGE_LOG_ERROR("Failed to read all the bytes of FILE_1");
    free(buffer1);
    return false;
}

#endif