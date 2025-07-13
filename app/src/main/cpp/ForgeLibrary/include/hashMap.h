#pragma once 
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef         char*                   byteArray;
typedef         unsigned long long      (hashFunction)      (const byteArray KEY, unsigned long long SIZE);
typedef         void*                   (memoryAllocate)    (unsigned long SIZE);
typedef         void                    (memoryDeallocate)  (void* MEM_ADDR);
typedef         int                     (memoryCompare)     (const void* PTR_1,   const void* PTR_2,        unsigned long SIZE);

typedef struct MapEntry
{
  char*                 key;
  unsigned long long    keySize;
  void*                 value;
  struct MapEntry*      next;
} MapEntry;

typedef struct HashMap
{
  unsigned long long    size;
  hashFunction*         hash;
  memoryAllocate*       allocator;
  memoryDeallocate*     deallocator;
  memoryCompare*        compare;
  MapEntry**            elements;
} HashMap;


FORGE_API bool      createHashMap           (HashMap* MAP, unsigned long long SIZE, hashFunction* HASH_FUNCTION, memoryAllocate* MALLOC, memoryDeallocate* FREE, memoryCompare* MEMCMP);

FORGE_API bool      destroyHashMap          (HashMap* MAP);

FORGE_API bool      hashMapInsert           (HashMap* MAP, const byteArray KEY,     unsigned long long KEY_SIZE, void* VALUE);

FORGE_API void*     hashMapGet              (HashMap* MAP, const byteArray KEY,     unsigned long long KEY_SIZE);

FORGE_API void*     hashMapRemove           (HashMap* MAP, const byteArray KEY,     unsigned long long KEY_SIZE);

#ifdef __cplusplus
}
#endif
