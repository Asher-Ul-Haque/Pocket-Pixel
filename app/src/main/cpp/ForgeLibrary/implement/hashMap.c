#include "../include/hashMap.h"
#include "../include/logger.h"
#include "../include/asserts.h"

static unsigned long long getIndex(HashMap* MAP, const byteArray KEY, unsigned long long KEY_SIZE)
{
  return (MAP->hash(KEY, KEY_SIZE)) % MAP->size;
}

static unsigned long long hash(const byteArray KEY, unsigned long long SIZE)
{
  const char* key = KEY;
  unsigned int hash = 0;
  while (*key) 
  {
    hash = (hash * 31) + *key++;
  }
  return hash % SIZE;
}

bool createHashMap(HashMap* MAP, unsigned long long SIZE, hashFunction* HASH_FUNCTION, memoryAllocate* MALLOC, memoryDeallocate* FREE, memoryCompare* MEMCMP)
{
  FORGE_ASSERT_MESSAGE(MAP != NULL, "Cannot initialize a null HashMap");
  FORGE_ASSERT_MESSAGE(SIZE > 0, "The size of a HashMap must be greater than 0");

  MAP->size         = SIZE;
  MAP->hash         = HASH_FUNCTION;
  MAP->allocator    = MALLOC;
  MAP->compare      = MEMCMP;
  MAP->deallocator  = FREE;

  if (HASH_FUNCTION == NULL)
  {
    FORGE_LOG_WARNING("NULL passed as hash function, going to use a simple hash");
    MAP->hash           = hash;    
  }
  if (MALLOC == NULL)
  {
    FORGE_LOG_WARNING("NULL passed as memory allocator, going with malloc from stdlib");
    MAP->allocator      = malloc;
  }
  if (FREE == NULL)
  {
    FORGE_LOG_WARNING("NULL passed as memory deallocator, going with free from stdlib");
    MAP->deallocator    = free;
  }
  if (MEMCMP == NULL)
  {
    FORGE_LOG_WARNING("NULL passed as memory compare, going with memcmp from stdlib");
    MAP->compare        = memcmp;
  }
  
  MAP->elements         = MAP->allocator(sizeof(MapEntry*) * MAP->size);
  if (MAP->elements == 0)
  {
    FORGE_LOG_ERROR("Memory Allocator failed for HashMap entries");
    return false;
  }

  return true;
}

bool destroyHashMap(HashMap* MAP)
{
  FORGE_ASSERT_MESSAGE(MAP != NULL, "Cannot destroy a NULL HashMap");
  FORGE_LOG_WARNING("Destroying HashMap");

  for (int i = 0; i < MAP->size; ++i)
  {
    while (MAP->elements[i])
    {
      MapEntry* entry = MAP->elements[i];
      MAP->elements[i] = MAP->elements[i]->next;
      MAP->deallocator(entry->key);
      MAP->deallocator(entry);
    }
  }

  MAP->deallocator(MAP->elements);

  FORGE_LOG_WARNING("The memory associated with the Map and Values are not owned by the Map. Cannot deallocate them");
  return true;
}

bool hashMapInsert(HashMap* MAP, const byteArray KEY, unsigned long long KEY_SIZE, void* VALUE)
{
  FORGE_ASSERT_MESSAGE(MAP != NULL, "Cannot insert into a null hashmap");
  FORGE_ASSERT_MESSAGE(KEY != NULL, "Cannot insert a NULL Key");
  FORGE_ASSERT_MESSAGE(KEY_SIZE > 0,"Cannot insert with a non-positive KEY_SIZE");

  if (VALUE == NULL)
  {
    FORGE_LOG_ERROR("Assiging a NULL value is the same as not assigning at all");
    return false;
  }

  unsigned long long  index = getIndex(MAP, KEY, KEY_SIZE);
  MapEntry*           head  = MAP->elements[index];

  while (head != NULL)
  {
    if (MAP->compare(head->key, KEY, KEY_SIZE) == 0)
    {
      head->value = VALUE;
      return true;
    }
    head = head->next;
  }

  MapEntry* entry       = MAP->allocator(sizeof(*entry));
  entry->value          = VALUE;
  entry->key            = MAP->allocator(KEY_SIZE);
  memcpy(entry->key, KEY, KEY_SIZE);

  entry->next           = MAP->elements[index];
  MAP->elements[index]  = entry;

  return true;
}

void* hashMapGet(HashMap* MAP, const byteArray KEY, unsigned long long KEY_SIZE)
{
  FORGE_ASSERT_MESSAGE(MAP != NULL, "MAP cannot be NULL");
  FORGE_ASSERT_MESSAGE(KEY != NULL, "KEY cannot be NULL");

  unsigned long long  index = getIndex(MAP, KEY, KEY_SIZE);
  MapEntry*           head  = MAP->elements[index];

  while (head != NULL && MAP->compare(head->key, KEY, KEY_SIZE) != 0)
  {
    head = head->next;
  }
  if (head == NULL)
  {
    return NULL;
  }
  return head->value;
}

void* hashMapRemove(HashMap* MAP, const byteArray KEY, unsigned long long KEY_SIZE)
{
  FORGE_ASSERT_MESSAGE(MAP != NULL, "MAP cannot be NULL");
  FORGE_ASSERT_MESSAGE(KEY != NULL, "KEY cannot be NULL");

  unsigned long long index = getIndex(MAP, KEY, KEY_SIZE);

  MapEntry* head = MAP->elements[index];
  MapEntry* prev = NULL;

  while (head != NULL && MAP->compare(head->key, KEY, KEY_SIZE) != 0)
  {
    prev = head;
    head = head->next;
  }

  if (head == NULL) return NULL;
  if (prev == NULL)
  {
    MAP->elements[index] = head->next;
  }
  else 
  {
    prev->next = head->next;
  }

  void* result = head->value;
  MAP->deallocator(head);

  return result;
}
