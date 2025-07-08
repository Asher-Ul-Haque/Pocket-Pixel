#include "../include/linearAlloc.h"
#include "../include/asserts.h"
#include "../include/logger.h"

void createLinearAllocator(unsigned long long TOTAL_SIZE, float RESIZE_FACTOR, void* MEMORY, LinearAllocator* ALLOCATOR) 
{
  FORGE_ASSERT_MESSAGE(ALLOCATOR     != NULL, "Cannot initialize a NULL ALLOACTOR");
  FORGE_ASSERT_MESSAGE(TOTAL_SIZE    >  0,    "Cannot make an ALLOACTOR with non-positive total size");
  FORGE_ASSERT_MESSAGE(RESIZE_FACTOR != 1,    "No point in assigning a RESIZE_FACTOR 1. If you want no resizing, pass 0");
  FORGE_ASSERT_MESSAGE(RESIZE_FACTOR >=0,     "Cannot make memory smaller. Use free memory instead of RESIZE_FACTOR");

  ALLOCATOR->totalSize    = TOTAL_SIZE;
  ALLOCATOR->allocated    = 0;
  ALLOCATOR->resizeFactor = RESIZE_FACTOR;

  if (RESIZE_FACTOR != 0)
  {
    FORGE_LOG_WARNING("The Linear Allocator will resize when it runs out of memory by a factor of %f", RESIZE_FACTOR);
  }
  if (MEMORY != NULL)
  {
    FORGE_LOG_WARNING("Memory is not null");
    ALLOCATOR->ownsMemory = false;
    ALLOCATOR->memory     = MEMORY;
  } 
  else 
  {
    FORGE_LOG_WARNING("The alloactor will own the memory, using malloc");
    ALLOCATOR->ownsMemory = true;
    ALLOCATOR->memory     = malloc(TOTAL_SIZE);
  }
}

void destroyLinearAllocator(LinearAllocator* ALLOCATOR) 
{
  FORGE_ASSERT_MESSAGE(ALLOCATOR    != NULL, "Cannot initialize a NULL ALLOACTOR");

  if (ALLOCATOR->ownsMemory && ALLOCATOR->memory) 
  {
    FORGE_LOG_WARNING("Freeing the allocator memory %p of %lld bytes", ALLOCATOR->memory, ALLOCATOR->allocated);
    free(ALLOCATOR->memory);
  } 
  ALLOCATOR->allocated  = 0;
  ALLOCATOR->memory     = 0;
  ALLOCATOR->totalSize  = 0;
  ALLOCATOR->ownsMemory = false;
}

void* linearAllocatorAllocate(LinearAllocator* ALLOCATOR, unsigned long long SIZE) 
{
  FORGE_ASSERT_MESSAGE(ALLOCATOR    != NULL, "Cannot initialize a NULL ALLOACTOR");
  FORGE_ASSERT_MESSAGE(ALLOCATOR->memory,    "ALLOCATOR has no memory");

  if (ALLOCATOR->allocated + SIZE > ALLOCATOR->totalSize) 
  {
    unsigned long long remaining = ALLOCATOR->totalSize - ALLOCATOR->allocated;

    if (ALLOCATOR->resizeFactor != 0)
    {
      FORGE_LOG_WARNING("LinearAllocator tried to allocate %lluB, only %lluB remaining.", SIZE, remaining);
      FORGE_LOG_WARNING("LinearAllocator is resizing by a factor of %f to size %lldB", ALLOCATOR->resizeFactor, (u64)(ALLOCATOR->totalSize * ALLOCATOR->resizeFactor));
      ALLOCATOR->memory = realloc(ALLOCATOR->memory, ALLOCATOR->totalSize * ALLOCATOR->resizeFactor);

      FORGE_ASSERT_MESSAGE(ALLOCATOR->memory != NULL, "LinearAllocator failed to resize. All memory lost");
      if (ALLOCATOR->memory == NULL)
      {
        FORGE_LOG_FATAL("LinearAllocator failed to ")
      }
    }
    else 
    {
      FORGE_LOG_ERROR("LinearAllocator tried to allocate %lluB, only %lluB remaining.", SIZE, remaining);
      return NULL;
    }
  }

  void* block = ((unsigned char*)ALLOCATOR->memory) + ALLOCATOR->allocated;
  ALLOCATOR->allocated += SIZE;
  return block;
}

void linearAllocFree(LinearAllocator* ALLOCATOR) 
{
  FORGE_ASSERT_MESSAGE(ALLOCATOR    != NULL, "Cannot initialize a NULL ALLOACTOR");
  FORGE_ASSERT_MESSAGE(ALLOCATOR->memory,    "ALLOCATOR has no memory");

  ALLOCATOR->allocated = 0;
  memset(ALLOCATOR->memory, 0, ALLOCATOR->totalSize);

  FORGE_LOG_WARNING("ALLOCATOR has freed all the memory");
}

void setLinearAllocatorResizeFactor(LinearAllocator* ALLOCATOR, float RESIZE_FACTOR)
{
  FORGE_ASSERT_MESSAGE(ALLOCATOR      != NULL, "Cannot channge resize factor of a NULL allocator");
  FORGE_ASSERT_MESSAGE(RESIZE_FACTOR  != 1,    "No point in assigning a RESIZE_FACTOR 1. If you want no resizing, pass 0");

  ALLOCATOR->resizeFactor = RESIZE_FACTOR;
}
