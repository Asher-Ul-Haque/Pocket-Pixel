#include "../include/orderedSet.h"
#include "../include/logger.h"
#include "../include/asserts.h"
#include "stdlib.h"
#include <string.h>


// - - - | Helper Functions On Nodes | - - - 


// - - - get height and balance - - - 

static i64 getHeight(AVLNode* NODE)  { return NODE ? NODE->height : 0; }
static i64 getBalance(AVLNode* NODE) { return NODE ? getHeight(NODE->left) - getHeight(NODE->right) : 0; }


// - - - create Node - - - 
static AVLNode* createNode(OrderedSet* SET, byteArray KEY)
{
  AVLNode* node = (AVLNode*) SET->allocator(sizeof(AVLNode));
  FORGE_ASSERT_MESSAGE(node, "[ORDERED SET] : Memory Allocation failed for AVL Node");

  node->key     = KEY;  
  node->left    = NULL;
  node->right   = NULL;
  node->height  = 1;

  memcpy(node->key, KEY, SET->keySize);
  return node;
}


// - - - rotation - - -

static AVLNode* rotateRight(AVLNode* Y)
{
  AVLNode* X    = Y->left;
  AVLNode* T2   = X->right;

  X->right      = Y;
  Y->left       = T2;

  Y->height     = 1 + ((getHeight(Y->left) > getHeight(Y->right)) ? 
                        getHeight(Y->left) : 
                        getHeight(Y->right));
  X->height     = 1 + ((getHeight(X->left) > getHeight(X->right)) ? 
                        getHeight(X->left) : 
                        getHeight(X->right));
  return X;
}

static AVLNode* rotateLeft(AVLNode* X)
{
  AVLNode* Y    = X->right;
  AVLNode* T2   = Y->left;

  Y->left       = X;
  X->right      = T2;

  X->height     = 1 + ((getHeight(X->left) > getHeight(X->right)) ? 
                        getHeight(X->left) : 
                        getHeight(X->right));
  Y->height     = 1 + ((getHeight(Y->left) > getHeight(Y->right)) ? 
                        getHeight(Y->left) : 
                        getHeight(Y->right));
  return Y;
}

// - - - find - - -

static AVLNode* findMinNode(AVLNode* NODE)
{
  while (NODE->left) NODE = NODE->left;
  return NODE;
}

static AVLNode* findMaxNode(AVLNode* NODE)
{
  while (NODE->right) NODE = NODE->right;
  return NODE;
}

static bool containsNode(AVLNode* NODE, memoryCompare* COMPARE, void* KEY, u64 KEY_SIZE)
{
  if (NODE == NULL) return false;

  i64 cmp = COMPARE(KEY, NODE->key, KEY_SIZE);
  if (cmp < 0) return containsNode(NODE->left,  COMPARE, KEY, KEY_SIZE);
  if (cmp > 0) return containsNode(NODE->right, COMPARE, KEY, KEY_SIZE);
  return true;
}


// - - - insert and delete - - - 

static AVLNode* insertNode(OrderedSet* SET, AVLNode* NODE, byteArray KEY)
{
  if (!NODE) return createNode(SET, KEY);

  // - - - compare
  i8 cmp = SET->compare(KEY, NODE->key, SET->keySize);
  if        (cmp == 0)  return NODE;
  else if   (cmp < 0)   NODE->left  = insertNode(SET, NODE->left, KEY);
  else if   (cmp > 0)   NODE->right = insertNode(SET, NODE->right, KEY);

  // - - - update height 
  NODE->height = 1 + (getHeight(NODE->left) > getHeight(NODE->right) ? 
                      getHeight(NODE->left) : 
                      getHeight(NODE->right));

  // - - - balance the nodes
  int balance = getBalance(NODE);
  if (balance > 1  && SET->compare(KEY, NODE->left->key, SET->keySize)  < 0) return rotateRight(NODE);
  if (balance < -1 && SET->compare(KEY, NODE->right->key, SET->keySize) > 0) return rotateLeft(NODE);
  if (balance > 1  && SET->compare(KEY, NODE->left->key, SET->keySize) > 0)
  {
    NODE->left = rotateLeft(NODE->left);
    return rotateRight(NODE);
  }
  if (balance < -1 && SET->compare(KEY, NODE->right->key, SET->keySize) < 0)
  {
    NODE->right = rotateRight(NODE->right);
    return rotateLeft(NODE);
  }

  return NODE;
}

typedef struct 
{
    AVLNode*  node;
    byteArray removedKey;
} DeleteResult;

static DeleteResult deleteNode(OrderedSet* SET, AVLNode* NODE, byteArray KEY)
{
  DeleteResult result = {NULL, NULL};
  
  if (NODE == NULL) return result;

  // - - - compare 
  int cmp = SET->compare(KEY, NODE->key, SET->keySize);
  
  if (cmp < 0) 
  {
    DeleteResult leftResult = deleteNode(SET, NODE->left, KEY);
    NODE->left              = leftResult.node;
    result.node             = NODE;
    result.removedKey       = leftResult.removedKey;
  }
  else if (cmp > 0) 
  {
    DeleteResult rightResult  = deleteNode(SET, NODE->right, KEY);
    NODE->right               = rightResult.node;
    result.node               = NODE;
    result.removedKey         = rightResult.removedKey;
  }
  else 
  {
    // - - - Found the node to delete, save the key to return it
    byteArray removedKey  = NODE->key;
    result.removedKey     = removedKey;
    
    if (NODE->left == NULL || NODE->right == NULL) 
    {
      AVLNode* temp = NODE->left ? NODE->left : NODE->right;

      if (temp == NULL) 
      {
        temp        = NODE;
        result.node = NULL;
      }
      else 
      {
        *NODE       = *temp;
        result.node = NODE;
      }
      SET->deallocator(temp);
    }
    else 
    {
      AVLNode* temp             = findMinNode(NODE->right);
      NODE->key                 = temp->key;
      
      DeleteResult rightResult  = deleteNode(SET, NODE->right, temp->key);
      NODE->right               = rightResult.node;
      
      result.node = NODE;
    }
  }

  if (result.node == NULL) return result;
  
  result.node->height = 1 + (getHeight(result.node->left) > getHeight(result.node->right) ? 
                        getHeight(result.node->left) :
                        getHeight(result.node->right));

  // - - - balance 
  i64 balance = getBalance(result.node);
  
  if (balance > 1 && getBalance(result.node->left) >= 0) 
  {
    result.node = rotateRight(result.node);
  }
  else if (balance < -1 && getBalance(result.node->right) <= 0) 
  {
    result.node = rotateLeft(result.node);
  }
  else if (balance > 1 && getBalance(result.node->left) < 0) 
  {
    result.node->left = rotateLeft(result.node->left);
    result.node       = rotateRight(result.node);
  }
  else if (balance < -1 && getBalance(result.node->right) > 0) 
  {
    result.node->right  = rotateRight(result.node->right);
    result.node         = rotateLeft(result.node);
  }

  return result;
}

static void deleteSetNodes(OrderedSet* SET, AVLNode* NODE)
{
  if (NODE)
  {
    deleteSetNodes(SET, NODE->left);
    deleteSetNodes(SET, NODE->right);
    SET->deallocator(NODE);
  }
}


// - - - traverse - - - 

static void inorderTraverse(AVLNode* NODE, orderedSetCallback CALLBACK)
{
  if (!NODE) return;
  inorderTraverse(NODE->left, CALLBACK);  // - - - left
  CALLBACK(NODE->key);                    // - - - root
  inorderTraverse(NODE->right, CALLBACK); // - - - right
}

static void preorderTraverse(AVLNode* NODE, orderedSetCallback CALLBACK)
{
  if (!NODE) return;
  CALLBACK(NODE->key);                     // - - - root
  preorderTraverse(NODE->left, CALLBACK);  // - - - left
  preorderTraverse(NODE->right, CALLBACK); // - - - right
}

static void postorderTraverse(AVLNode* NODE, orderedSetCallback CALLBACK)
{
  if (!NODE) return;
  postorderTraverse(NODE->left, CALLBACK);  // - - - left
  postorderTraverse(NODE->right, CALLBACK); // - - - right
  CALLBACK(NODE->key);                      // - - - root
}


// - - - | Ordered Set Functions | - - - 


// - - - create and destroy - - - 

void createOrderedSet(OrderedSet* SET, u64 KEY_SIZE, memoryCompare* COMPARE, memoryAllocate* MALLOC, memoryDeallocate* FREE)
{
  FORGE_ASSERT_MESSAGE(SET,      "[ORDERED SET] : Cannot initialize a null OrderedSet");
  FORGE_ASSERT_MESSAGE(KEY_SIZE, "[ORDERED SET] : Cannot have a key size of less than 1 byte");

  SET->compare      = COMPARE;
  SET->allocator    = MALLOC;
  SET->deallocator  = FREE;
  SET->keySize      = KEY_SIZE;
  SET->root         = NULL;


  if (COMPARE == NULL)
  {
    FORGE_LOG_WARNING("[ORDERED SET] : No memory comparison function passed. Will use 'memcmp' from stdlib");
    SET->compare = memcmp;
  }
  if (MALLOC == NULL)
  {
    FORGE_LOG_WARNING("[ORDERED SET] : No memory allocation function passed. Will use 'malloc' from stdlib");
    SET->allocator = malloc;
  }
  if (FREE == NULL)
  {
    FORGE_LOG_WARNING("[ORDERED SET] : No memory deallocation function passed. Will use 'free' from stdlib");
    SET->deallocator = free;
  }
}

void destroyOrderedSet(OrderedSet* SET)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot destroy a NULL ordered set");
  deleteSetNodes(SET, SET->root);
  SET->root         = NULL;
  SET->deallocator  = NULL;
  SET->allocator    = NULL;
  SET->compare      = NULL;
  SET->keySize      = -1;
  SET->size         = -1;
}

void clearOrderedSet(OrderedSet* SET)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot clear a NULL ordered set");
  deleteSetNodes(SET, SET->root);
  SET->root         = NULL;
  SET->size         = 0;
}


// - - - metadata - - - 

u64 getOrderedSetSize(OrderedSet* SET)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot get size of a NULL ordered Set");
  return SET->size;
}

u64 getOrderedSetHeight(OrderedSet* SET)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot get height of a NULL ordered Set");
  return  SET->root ? SET->root->height : 0;
}


// - - - insert and delete - - - 

void orderedSetInsert(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot insert in a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot insert a NULL key");

  SET->root = insertNode(SET, SET->root, KEY);
  SET->size++;
}

byteArray orderedSetRemove(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot remove from a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot remove a NULL key");

  u64 oldSize         = SET->size;
  DeleteResult result = deleteNode(SET, SET->root, KEY);
  SET->root           = result.node;

  if (oldSize > SET->size) SET->size--;
  
  return result.removedKey;
}

bool orderedSetContains(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot remove from a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot remove a NULL key");

  return containsNode(SET->root, SET->compare, KEY, SET->keySize);
}


// - - - Predecessor Successor - - - 

byteArray orderedSetPredecessor(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot find successor in a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot find successor of a NULL key");

  AVLNode* current    = SET->root;
  AVLNode* successor  = NULL;

  while (current)
  {
    i32 cmp = SET->compare(KEY, current->key, SET->keySize);
    if (cmp < 0) // - - - Move left, but mark current as potential successor
    {
      successor = current;
      current = current->left;
    }
    else if (cmp > 0)
    {
      current = current->right; // - - - Move right, ignore successor
    }
    else
    {
      // - - - Case: If there's a right subtree, get the min node in right subtree
      if (current->right)
      {
        successor = findMinNode(current->right);
      }
      break;
    }
  }

  return successor ? successor->key : NULL;
}

byteArray orderedSetSuccessor(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot find predecessor in a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot find predecessor of a NULL key");

  AVLNode* current     = SET->root;
  AVLNode* predecessor = NULL;

  while (current)
  {
    i32 cmp = SET->compare(KEY, current->key, SET->keySize);
    if (cmp > 0) // - - - Move right, but mark current as potential predecessor
    {
      predecessor = current;
      current = current->right;
    }
    else if (cmp < 0)
    {
      current = current->left; // - - - Move left, ignore predecessor
    }
    else
    {
      // Case: If there's a left subtree, get the max node in left subtree
      if (current->left)
      {
        predecessor = findMaxNode(current->left);
      }
      break;
    }
  }

  return predecessor ? predecessor->key : NULL;
}


// - - - Traversal - - - 

void orderedSetTraverse(OrderedSet* SET, orderedSetCallback CALLBACK, TraversalType TYPE)
{
  FORGE_ASSERT_MESSAGE(SET,       "[ORDERED SET] : Cannot traverse a NULL ordered set");
  FORGE_ASSERT_MESSAGE(CALLBACK,  "[ORDERED SET] : Cannot traverse with a NULL callback");
  FORGE_ASSERT_MESSAGE(TYPE >= TRAVERSAL_IN_ORDER && TYPE < TRAVERSAL_COUNT,      "[ORDERED SET] : Can only do inorder, preorder or postorder traverse");

  switch (TYPE)
  {
    case TRAVERSAL_PRE_ORDER  : preorderTraverse  (SET->root, CALLBACK); break;
    case TRAVERSAL_IN_ORDER   : inorderTraverse   (SET->root, CALLBACK); break;
    case TRAVERSAL_POST_ORDER : postorderTraverse (SET->root, CALLBACK); break;
    default                   : break; 
  }
}

void createOrderedSetIter(OrderedSet* SET, OrderedSetIterator* ITERATOR)
{
  FORGE_ASSERT_MESSAGE(SET,       "[ORDERED SET] : Cannot iterate a NULL ordered set");
  FORGE_ASSERT_MESSAGE(ITERATOR,  "[ORDERED SET] : Cannot iterate an ordered set with a NULL iterator");

  ITERATOR->top = -1;
  AVLNode* current = SET->root;

  while (current)
  {
    ITERATOR->stack[++(ITERATOR->top)] = current;
    current = current->left;
  }
}

byteArray orderedSetIterNext(OrderedSet* SET, OrderedSetIterator* ITERATOR)
{
  FORGE_ASSERT_MESSAGE(SET,       "[ORDERED SET] : Cannot iterate a NULL ordered set");
  FORGE_ASSERT_MESSAGE(ITERATOR,  "[ORDERED SET] : Cannot iterate an ordered set with a NULL iterator");

  if (ITERATOR->top < 0) return NULL;

  AVLNode* current = ITERATOR->stack[(ITERATOR->top)--];
  byteArray retVal = current->key;

  current = current->right;
  while (current)
  {
    ITERATOR->stack[++(ITERATOR->top)] = current;
    current = current->left;
  }

  return retVal;
}


// - - - find - - - 

byteArray orderedSetFindSmallestAtleast(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot seacrh in a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot seacrh with a NULL key");

  AVLNode* current = SET->root;
  AVLNode* bestFit = NULL;

  while (current)
  {
    i32 cmp = SET->compare(KEY, current->key, SET->keySize);

    // - - - if the current node is larger or equal, it might be the answer
    if (cmp <= 0)
    {
      bestFit = current;
      current = current->left;    // - - - move to left, to find something smaller
    }
    else current = current->right; // - - - not greater than yet
  }

  return bestFit ? bestFit->key : NULL;
}

byteArray orderedSetFindGreatestSmallerThan(OrderedSet* SET, byteArray KEY)
{
  FORGE_ASSERT_MESSAGE(SET, "[ORDERED SET] : Cannot seacrh in a NULL ordered set");
  FORGE_ASSERT_MESSAGE(KEY, "[ORDERED SET] : Cannot seacrh with a NULL key");

  AVLNode* current = SET->root;
  AVLNode* bestFit = NULL;

  while (current)
  {
    i32 cmp = SET->compare(KEY, current->key, SET->keySize);

    // - - - if the current node is smaller, it might be the answer
    if (cmp > 0)
    {
      bestFit = current;
      current = current->right;    // - - - move to left, to find something smaller
    }
    else current = current->left;  // - - - not greater than yet
  }

  return bestFit ? bestFit->key : NULL;
}
