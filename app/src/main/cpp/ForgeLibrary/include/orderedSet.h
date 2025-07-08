#pragma once 
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif

// - - - Allocate and Deallocate functions
typedef char* byteArray;
typedef i32   (memoryCompare)      (const void* PTR_1,   const void* PTR_2,        unsigned long SIZE);
typedef void  (memoryDeallocate)   (void*           MEM_ADDR);
typedef void* (memoryAllocate)     (unsigned long   SIZE);

// - - - structures for the AVL tree
typedef void (*orderedSetCallback) (const byteArray KEY);

typedef enum
{
  TRAVERSAL_IN_ORDER,
  TRAVERSAL_POST_ORDER,
  TRAVERSAL_PRE_ORDER,
  TRAVERSAL_COUNT
} TraversalType;

typedef struct AVLNode
{
  byteArray          key;
  struct AVLNode*    left;
  struct AVLNode*    right;
  i32                height;
} AVLNode;

typedef struct OrderedSetIterator
{
  AVLNode* stack[64];
  i32 top;
} OrderedSetIterator;

typedef struct OrderedSet
{
  AVLNode*            root;
  u64                 keySize;
  u64                 size;
  memoryCompare*      compare;
  memoryAllocate*     allocator;
  memoryDeallocate*   deallocator;
} OrderedSet;


// - - - Functions on the AVL tree - - - 

// - - - Create and Destroy
FORGE_API void      createOrderedSet      (OrderedSet* SET, u64 KEY_SIZE, memoryCompare* COMPARE, memoryAllocate* MALLOC, memoryDeallocate* FREE);
FORGE_API void      destroyOrderedSet     (OrderedSet* SET);
FORGE_API void      clearOrderedSet       (OrderedSet* SET);
FORGE_API u64       getOrderedSetSize     (OrderedSet* SET);
FORGE_API u64       getOrderedSetHeight   (OrderedSet* SET);

// - - - Insert, Remove, Search
FORGE_API void      orderedSetInsert      (OrderedSet* SET, byteArray KEY);
FORGE_API byteArray orderedSetRemove      (OrderedSet* SET, byteArray KEY);
FORGE_API bool      orderedSetContains    (OrderedSet* SET, byteArray KEY);

// - - - Successor and Predecessor
FORGE_API byteArray orderedSetSuccessor   (OrderedSet* SET, byteArray KEY);
FORGE_API byteArray orderedSetPredecessor (OrderedSet* SET, byteArray KEY);

// - - - Traversal
FORGE_API void      createOrderedSetIter  (OrderedSet* SET, OrderedSetIterator* ITERATOR);
FORGE_API byteArray orderedSetIterNext    (OrderedSet* SET, OrderedSetIterator* ITERATOR);
FORGE_API void      orderedSetTraverse    (OrderedSet* SET, orderedSetCallback  CALLBACK, TraversalType TYPE);

// - - - Find 
FORGE_API byteArray orderedSetFindSmallestAtleast    (OrderedSet* SET, byteArray KEY);
FORGE_API byteArray orderedSetFindGreatestSmallerThan(OrderedSet* SET, byteArray KEY);


#ifdef __cplusplus
}
#endif
