#include "../include/objectPool.h"
#include "../include/asserts.h"
#include "../include/logger.h"
#include <stdlib.h>

void createObjectPool(u64 TOTAL_CAPACITY, u64 OBJECT_SIZE, void* MEMORY, ObjectPool* POOL)
{
  FORGE_ASSERT_MESSAGE(POOL,                           "[OBJECT POOL] : Cannot initialize a NULL Object Pool");
  FORGE_ASSERT_MESSAGE(TOTAL_CAPACITY > 0,             "[OBJECT POOL] : Must be able to store atleast 1 object");

  POOL->capacity        = TOTAL_CAPACITY;
  POOL->memory          = MEMORY;
  POOL->objectSize      = (OBJECT_SIZE >= sizeof(u64)) ? OBJECT_SIZE : sizeof(u64);

  if (OBJECT_SIZE < sizeof(u64))
  {
    FORGE_LOG_WARNING("[OBJECT POOL] : is really efficient for lots of big objects");
    FORGE_LOG_WARNING("\t It works on an internal linked list structure that requires all the objects to be atleast %lu bytes in size", sizeof(u64)); 
    FORGE_LOG_WARNING("\t so %lu byte padding has been added to your objects", sizeof(u64) - OBJECT_SIZE);
    FORGE_LOG_WARNING("\t this is causing %.2f percent of the POOL to be wasted", 100.0f * (POOL->objectSize - OBJECT_SIZE) / (f32)POOL->objectSize);
  }
  if (MEMORY != NULL)
  {
    FORGE_LOG_WARNING("[OBJECT POOL] : Memory is not null, we do not own the memory");
  }
  else 
  {
    FORGE_LOG_WARNING("[OBJECT POOL] : Object Pool will own the memory, using malloc");
    POOL->memory     = malloc(TOTAL_CAPACITY * POOL->objectSize);
    FORGE_ASSERT_MESSAGE(POOL->memory, "[OBJECT POOL] : Failed to malloc memory for the object pool");
  }

  // - - - at the beginning the memory, store the linked list of free pointers
  POOL->freeListOffset = 0;
  for (u64 i = 0; i < TOTAL_CAPACITY - 1; ++i)
  {
    u64* whereToStore = (u64*)((u8*)POOL->memory + (i * POOL->objectSize));
    u64  whatToStore  = (i + 1) * POOL->objectSize; 
    *whereToStore     = whatToStore;  
  }

  // - - - the last one doesnt point to anything, so -1
  u64* lastOne = (u64*)((u8*)POOL->memory + ((TOTAL_CAPACITY - 1) * POOL->objectSize));
  *lastOne = (u64)-1;
}

void* takeObject(ObjectPool* POOL)
{
  FORGE_ASSERT_MESSAGE(POOL,         "[OBJECT POOL] : Cannot get an Object from a NULL Object Pool");
  FORGE_ASSERT_MESSAGE(POOL->memory, "[OBJECT POOL] : no memory");

  if (POOL->freeListOffset == -1)
  {
    FORGE_LOG_ERROR("[OBJECT POOL] : ran out of memory!"); 
    return NULL; 
  }

  // - - - make a pointer from the offset
  u8* ptr = ((u8*) POOL->memory) + POOL->freeListOffset;

  // - - - move the free list to the next free object, which is the value of ptr
  POOL->freeListOffset = *(u64*) ptr;

  // - - - return the void*
  return (void*) ptr;
}

void returnObject(ObjectPool* POOL, void* OBJECT)
{
  FORGE_ASSERT_MESSAGE(POOL,   "[OBJECT POOL] : Cannot return an object to a NULL object pool");
  FORGE_ASSERT_MESSAGE(OBJECT, "[OBJECT POOL] : Cannot return NULL to an Object Pool");

  // - - - make offset from pointer 
  u64 offset = (u8*)OBJECT - (u8*)POOL->memory;

  // - - - check if the offset is valid
  FORGE_ASSERT_MESSAGE(offset % POOL->objectSize == 0,              "[OBJECT POOL] : Invalid object address, not aligned to object size");
  FORGE_ASSERT_MESSAGE(offset / POOL->objectSize < POOL->capacity,  "[OBJECT POOL] : Invalid object address, out of bounds (more than memory)");
  FORGE_ASSERT_MESSAGE(offset >= 0,                                 "[OBJECT POOL] : Invalid object address, out of bounds (less than 0)");


  // - - - store the old free list head at this place 
  u64*  whereToStore    = (u64*) OBJECT;
  u64   whatToStore     = POOL->freeListOffset;
  *whereToStore         = whatToStore;

  // - - - move the free list head to this return object 
  POOL->freeListOffset = offset;
}

void destroyObjectPool(ObjectPool* POOL)
{
  FORGE_ASSERT_MESSAGE(POOL, "[OBJECT POOL] : Cannot delete a NULL object Pool");
  free(POOL->memory);
  POOL->objectSize      = -1;
  POOL->capacity        = -1;
  POOL->freeListOffset  = -1;
}

void visualizeObjectPool(ObjectPool* POOL) 
{
  FORGE_ASSERT_MESSAGE(POOL, "[OBJECT_POOL] : Cannot visualize a NULL object POol");

  u64 freeCount = 0, usedCount = 0;
  u64 offset = POOL->freeListOffset;
  int isFree[POOL->capacity]; // Array to track free/used objects

  for (u64 i = 0; i < POOL->capacity; i++) isFree[i] = 0;

  // Traverse free list and mark free objects
  while (offset != (u64)-1) {
      u64 index = offset / POOL->objectSize;
      isFree[index] = 1;
      freeCount++;
      offset = *(u64 *)(POOL->memory + offset);
  }

  usedCount = POOL->capacity - freeCount;


  FORGE_LOG_INFO("Used: %llu | Free: %llu", usedCount, freeCount);
}
