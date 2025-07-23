#pragma once
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct linearAllocator 
{
  unsigned long long    totalSize;
  unsigned long long    allocated;
  float                 resizeFactor;
  void*                 memory;
  bool                  ownsMemory;
} LinearAllocator;

FORGE_API void  createLinearAllocator(unsigned long long TOTAL_SIZE, float RESIZE_FACTOR, void* MEMORY, LinearAllocator* ALLOCATOR);

FORGE_API void  destroyLinearAllocator(LinearAllocator* ALLOCATOR);

FORGE_API void* linearAllocatorAllocate(LinearAllocator* ALLOCATOR,     unsigned long long SIZE);

FORGE_API bool  linearAllocatorRemove(LinearAllocator* ALLOCATOR,     unsigned long long SIZE);

FORGE_API void  linearAllocZero(LinearAllocator* ALLOCATOR); 

FORGE_API void  linearAllocReset(LinearAllocator* ALLOCATOR); 

FORGE_API void  setLinearAllocatorResizeFactor(LinearAllocator* ALLOCATOR, float RESIZE_FACTOR);

#ifdef __cplusplus
}
#endif
