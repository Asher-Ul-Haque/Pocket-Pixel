#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct linearAllocator 
{
  unsigned long long    totalSize;
  unsigned long long    allocated;
  void*                 memory;
  bool                  ownsMemory;
} LinearAllocator;

FORGE_API void  createLinearAllocator(unsigned long long TOTAL_SIZE,    void* MEMORY, LinearAllocator* ALLOCATOR);

FORGE_API void  destroyLinearAllocator(LinearAllocator* ALLOCATOR);

FORGE_API void* linearAllocatorAllocate(LinearAllocator* ALLOCATOR,     unsigned long long SIZE);

FORGE_API void  linearAllocFree(LinearAllocator* ALLOCATOR); 

#ifdef __cplusplus
}
#endif
