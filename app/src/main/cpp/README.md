
# Forge Utilities Library

A simple C library for threadPooling, logging, filesystem operations, memory management, testing, asserting, TODOs etc


1. [Introduction](#introduction)
2. [Log System](#log-system)
   - [Log Macros](#log-macros)
   - [Examples](#examples)
   - [How to Customize Logging](#how-to-customize-logging)
3. [Assertions](#assertions)
   - [Assert Macros](#assert-macros)
   - [Examples](#examples-1)
   - [To Toggle Assertions](#to-toggle-assertions)
4. [File System Operations](#file-system-operations)
   - [File Operations](#file-operations)
   - [Examples](#examples-2)
   - [To Toggle Using Syscalls for High Level](#to-toggle-using-syscalls-for-high-level)
5. [Hash Map](#hash-map)
   - [Functions](#functions)
   - [Customizing the Hashmap](#customizing-the-hashmap)
   - [Examples](#examples-3)
6. [Linear Allocator](#linear-allocator)
   - [Functions](#functions-1)
   - [Examples](#examples-4)
7. [Object Pool](#object-pool)
   - [Functions](#functions-2)
   - [Examples](#examples-5)
8. [OrderedSet (AVL Tree)](#orderedset-avl-tree)
   - [Functions](#functions-3)
9. [TestManager and Expect](#testmanager-and-expect)
   - [Functions](#functions-4)
   - [Examples](#examples-6)
10. [ThreadPool](#threadpool)
    - [Functions](#functions-5)
    - [Examples](#examples-7)
11. [Building and Linking](#building-and-linking)
12. [License](#license)

---

## Introduction

The Forge Utilities Library provides a set of tools for logging, assertions, and file operations, useful for debugging, error handling, and managing file I/O in C projects.

---

## Log System

The log system provides macros for printing log messages with various severity levels: Fatal, Error, Warning, Info, Debug, and Trace. On android, it connects to logcat

### Log Macros

| Macro                | Description                                      |
|----------------------|--------------------------------------------------|
| `FORGE_LOG_FATAL`    | Logs a fatal error and terminates execution.     |
| `FORGE_LOG_ERROR`    | Logs a critical error.                           |
| `FORGE_LOG_WARNING`  | Logs a warning if enabled.                       |
| `FORGE_LOG_INFO`     | Logs informational messages if enabled.          |
| `FORGE_LOG_DEBUG`    | Logs debug messages if enabled.                  |
| `FORGE_LOG_TRACE`    | Logs trace messages for detailed step tracking.  |

### Examples

```c
#include "logger.h"

void exampleLogging() 
{
    FORGE_LOG_FATAL("This is a fatal error!");
    FORGE_LOG_ERROR("An error occurred: %d", -1);
    FORGE_LOG_WARNING("Warning: Something might be wrong.");
    FORGE_LOG_INFO("Info: System is running as expected.");
    FORGE_LOG_DEBUG("Debug: Variable value is %d", 42);
    FORGE_LOG_TRACE("Trace: Entering function %s", __func__);
}
```

### How to customize logging-
Logging can be turned off or not by defining some macros such as
```c
#define LOG_WARNING_ENABLED   	0 // - - - warning logs are disabled
#define LOG_INFO_ENABLED  	1 // - - - info logs are enabled
```

---

## Assertions

Assertions help enforce conditions and identify bugs by halting the program when an assertion fails.

### Assert Macros

| Macro                     | Description                                               |
|---------------------------|-----------------------------------------------------------|
| `FORGE_ASSERT`            | Checks an expression and halts the program if it fails.   |
| `FORGE_ASSERT_MESSAGE`    | Similar to `FORGE_ASSERT` but allows for custom messages. |
| `FORGE_ASSERT_DEBUG`      | Debug-only assertion, active when debug logging is enabled. |
| `TODO`   		    | Prints the place where TODO occurs when it is hit. |
| `TODO_COMMENT`            | `TODO` but allows for a custom comment. |

### Examples

```c
#include "asserts.h"

void exampleAssertions() 
{
    int x = 5;
    FORGE_ASSERT(x > 0); // Passes
    FORGE_ASSERT_MESSAGE(x < 0, "x should be less than 0"); // Fails with a message

    #if LOG_DEBUG_ENABLED == 1
    FORGE_ASSERT_DEBUG(x == 5); // Debug assertion, passes in debug mode
    #endif
    
    TODO; // - - - if the program gets to here, it will halt
}
```

### To Toggle Assertions

```c
#define FORGE_ASSERTS_ENABLED 0
```


---

## File System Operations

The file system module provides functions for working with files, such as checking file existence, reading, writing, and getting file sizes.

### File Operations

| Function               | Description                                      |
|------------------------|--------------------------------------------------|
| `getFileSize`          | Retrieves the size of a file.                   |
| `fileExists`           | Checks if a file exists at a given path.        |
| `openFile`             | Opens a file in a given mode (read, write, etc.). |
| `closeFile`            | Closes an open file.                            |
| `readFileLine`         | Reads a single line from the file.              |
| `writeFileLine`        | Writes a line of text to the file.              |
| `readFile`             | Reads a specific number of bytes from a file.   |
| `writeFile`            | Writes data to a file.                          |
| `setCursor`            | Moves the file cursor to a specific position.   |
| `offsetCursor`         | Moves the file cursor by a specified offset.    |
| `getCursor`            | Retrieves the current position of the file cursor. |

### Examples

```c
#include "filesystem.h"
#include "logger.h"

void exampleFilesystem() 
{
    File myFile;
    
    if (openFile("test.txt", FILE_MODE_READ, false, &myFile)) 
    {
        char* line;
        while (readFileLine(&myFile, &line)) 
        {
            FORGE_LOG_INFO("Line: %s", line);
        }
        closeFile(&myFile);
    } 
    else 
    {
        FORGE_LOG_ERROR("Failed to open file!");
    }

    if (fileExists("test.txt")) 
    {
        FORGE_LOG_INFO("File exists!");
    }
}
```

### To Toggle Using Syscalls for High Level

```c
#define USE_SYSCALLS 0
```

---

## Hash Map

The **Hash Map** module provides a simple implementation of a hash map (or dictionary) to store key-value pairs efficiently. The module supports customizable hash functions, memory allocation, deallocation, and comparison functions to suit your needs.

| Function               | Description                                      |
|------------------------|--------------------------------------------------|
| `createHashMap`        | Initializes a new hash map with the specified size and functions. | 
| `destroyHashMap`       | Destroys a hash map and frees all associated memory.        |
| `hashMapInsert`        | Inserts a key-value pair into the hash map. |
| `hashMapGet`           | Retrieves the value associated with a key                            |
| `hashMapRemove`        | Removes a key-value pair from the hash map              |


### Customizing the Hashmap
```c
typedef         char*                   byteArray;
typedef         unsigned long long      (hashFunction)      (const byteArray KEY, unsigned long long SIZE);
typedef         void*                   (memoryAllocate)    (unsigned long SIZE);
typedef         void                    (memoryDeallocate)  (void* MEM_ADDR);
typedef         int                     (memoryCompare)     (const void* PTR_1, const void* PTR_2, unsigned long SIZE);
```

### Examples
```c
#include "hashMap.h"
#include <stdio.h>

unsigned long long simpleHash(const byteArray key, unsigned long long size) {
    unsigned long long hash = 0;
    for (unsigned long long i = 0; i < size; ++i) {
        hash = (hash * 31) + key[i];
    }
    return hash;
}

int main() {
    HashMap map;
    if (!createHashMap(&map, 16, simpleHash, malloc, free, memcmp)) {
        printf("Failed to create hash map\n");
        return -1;
    }

    // Insert data
    int value = 42;
    byteArray key = "testKey";
    if (hashMapInsert(&map, key, strlen(key), &value)) {
        printf("Key inserted successfully\n");
    }

    // Retrieve data
    int* retrievedValue = (int*)hashMapGet(&map, key, strlen(key));
    if (retrievedValue) {
        printf("Retrieved value: %d\n", *retrievedValue);
    } else {
        printf("Key not found\n");
    }

    // Remove data
    hashMapRemove(&map, key, strlen(key));

    // Destroy hash map
    destroyHashMap(&map);
    
    return 0;
}

```

---

## Linear Allocator
The Linear Allocator provides a simple and efficient way to allocate and manage memory in a linear fashion. It is particularly useful for scenarios where memory is allocated in a predictable, sequential order.

### Functions
| Function                 | Description                                      |
|--------------------------|--------------------------------------------------|
| `createLinearAllocator`  | Initializes a linear allocator. | 
| `destroyLinearAllocator` | Destroys the allocator and frees any owned memory. |
| `linearAllocatorAllocate`| Allocates a block of memory from the allocator. |
| `linearAllocFree`        | Frees all allocated memory in the allocator. |
| `setLinearAllocatorResizeFactor` | Sets the resize factor for the allocator. |

### Examples
```c
#include "linearAlloc.h"
#include <stdio.h>
#include <string.h>

// Example structure to allocate memory for
typedef struct ExampleStruct {
    int id;
    char name[50];
} ExampleStruct;

int main() {
    // Create the linear allocator with an initial size of 1024 bytes and resize factor of 1.5
    LinearAllocator allocator;
    createLinearAllocator(1024, 1.5, NULL, &allocator);

    // Allocate memory for an ExampleStruct
    ExampleStruct* example1 = (ExampleStruct*)linearAllocatorAllocate(&allocator, sizeof(ExampleStruct));
    if (example1) {
        example1->id = 1;
        strcpy(example1->name, "Example One");
        printf("Allocated ExampleStruct: ID = %d, Name = %s\n", example1->id, example1->name);
    }

    // Allocate another ExampleStruct
    ExampleStruct* example2 = (ExampleStruct*)linearAllocatorAllocate(&allocator, sizeof(ExampleStruct));
    if (example2) {
        example2->id = 2;
        strcpy(example2->name, "Example Two");
        printf("Allocated ExampleStruct: ID = %d, Name = %s\n", example2->id, example2->name);
    }

    // Free all allocated memory
    linearAllocFree(&allocator);

    // Destroy the allocator and free the memory it owns
    destroyLinearAllocator(&allocator);

    return 0;
}

```

---

## Object Pool
The **Object Pool** module provides an efficient memory management strategy for reusing objects instead of repeatedly allocating and deallocating memory. This is especially useful in scenarios where objects are frequently created and destroyed.

### Functions
| Function                 | Description                                      |
|--------------------------|--------------------------------------------------|
| `createObjectPool`  | Initializes an Object Pool with a given capacity and object size. | 
| `takeObject` | Allocates an object from the pool. |
| `returnObject`| Releases the object back to the pool. |
| `destroyObjectPool`        | Destroys the object pool and frees associated memory. |

### Examples
```c
#include "objectPool.h"
#include <stdio.h>
#include <string.h>

// Example structure to be managed by the object pool
typedef struct ExampleObject {
    int id;
    char name[50];
} ExampleObject;

int main() {
    // Create the object pool with a capacity of 10 objects, each of size ExampleObject
    ObjectPool pool;
    createObjectPool(10, sizeof(ExampleObject), NULL, &pool);

    // Take an object from the pool
    ExampleObject* obj1 = (ExampleObject*)takeObject(&pool);
    if (obj1) {
        obj1->id = 1;
        strcpy(obj1->name, "Object One");
        printf("Allocated Object: ID = %d, Name = %s\n", obj1->id, obj1->name);
    }

    // Take another object from the pool
    ExampleObject* obj2 = (ExampleObject*)takeObject(&pool);
    if (obj2) {
        obj2->id = 2;
        strcpy(obj2->name, "Object Two");
        printf("Allocated Object: ID = %d, Name = %s\n", obj2->id, obj2->name);
    }

    // Return the objects back to the pool
    returnObject(&pool, obj1);
    returnObject(&pool, obj2);

    // Visualize the pool state (for debugging)
    visualizeObjectPool(&pool);

    // Destroy the pool and free the memory
    destroyObjectPool(&pool);

    return 0;
}
```

---

## OrderedSet (AVL Tree)
An **Ordered Set** is a data structure that holds unique elements in sorted order. This implementation uses an **AVL Tree**, a self-balancing binary search tree, to ensure that insertions, deletions, and lookups are all efficient, with time complexities of O(log N)

### Functions

| Function                                                     | Description                                                                 |
|--------------------------------------------------------------|-----------------------------------------------------------------------------|
| `createOrderedSet(OrderedSet* SET, u64 KEY_SIZE, memoryCompare* COMPARE, memoryAllocate* MALLOC, memoryDeallocate* FREE)` | Initializes an ordered set with a given key size, comparison function, and memory management functions. |
| `destroyOrderedSet(OrderedSet* SET)`                         | Destroys the ordered set and frees any allocated memory.                    |
| `clearOrderedSet(OrderedSet* SET)`                           | Clears all elements from the ordered set.                                   |
| `getOrderedSetSize(OrderedSet* SET)`                         | Returns the number of elements in the ordered set.                          |
| `getOrderedSetHeight(OrderedSet* SET)`                       | Returns the height of the AVL tree.                                         |
| `orderedSetInsert(OrderedSet* SET, byteArray KEY)`           | Inserts a key into the ordered set.                                         |
| `orderedSetRemove(OrderedSet* SET, byteArray KEY)`           | Removes a key from the ordered set.                                         |
| `orderedSetContains(OrderedSet* SET, byteArray KEY)`         | Checks if a key exists in the ordered set.                                  |
| `orderedSetSuccessor(OrderedSet* SET, byteArray KEY)`        | Finds the smallest key greater than the given key.                          |
| `orderedSetPredecessor(OrderedSet* SET, byteArray KEY)`      | Finds the largest key smaller than the given key.                           |
| `orderedSetFindSmallestAtleast(OrderedSet* SET, byteArray KEY)` | Finds the smallest key that is greater than or equal to the given key.    |
| `orderedSetFindGreatestSmallerThan(OrderedSet* SET, byteArray KEY)` | Finds the greatest key that is smaller than the given key.                |
| `createOrderedSetIter(OrderedSet* SET, OrderedSetIterator* ITERATOR)` | Initializes an iterator for traversing the ordered set.                    |
| `orderedSetIterNext(OrderedSet* SET, OrderedSetIterator* ITERATOR)`   | Returns the next key in the iteration.                                      |
| `orderedSetTraverse(OrderedSet* SET, orderedSetCallback CALLBACK, TraversalType TYPE)` | Traverses the ordered set in a specific order (in-order, pre-order, post-order). |

---

## TestManager and Expect
Register and run test cases as functions. If a segfault occurs, the test doesnt crash.

### Functions

| Function                                           | Description                                                                 |
|----------------------------------------------------|-----------------------------------------------------------------------------|
| `registerTest` | Registers a test function with a description. The test function is of type  `TEST`, which returns a `u8` value. |
| `runTests`                                       | Runs all registered tests, including those that check for segmentation faults. |
| `expectShouldBe`              | Checks if the expected value equals the actual value. If not, logs an error with file and line details. |
| `expectStringToBe`            | Compares two strings. Logs an error if they are not equal, along with file and line details. |
| `expectShouldNotBe`           | Checks if the expected value does not equal the actual value. If they are equal, logs an error with file and line details. |
| `expectFloatToBe`             | Compares two float values within a small tolerance (0.001). Logs an error if they differ beyond the tolerance. |
| `expectToBeTrue`                        | Checks if the actual value is `true`. Logs an error if it is not. |
| `expectToBeNULL`                        | Checks if the actual value is `NULL`. Logs an error if it is not. |
| `expectToBeNotNULL`                     | Checks if the actual value is not `NULL`. Logs an error if it is. |
| `expectToBeFalse`                       | Checks if the actual value is `false`. Logs an error if it is not. |


### Examples
```c
#include "testManager.h"       // For test registration and running
#include "expect.h"     // For expectations

// Sample function to be tested
int add(int a, int b) {
    return a + b;
}

// Test function for 'add'
u8 testAddFunction() {
    int result = add(2, 3);
    
    // Use expect macros to validate the function
    expectShouldBe(5, result);  // Expect 2 + 3 to equal 5

    result = add(-1, 1);
    expectShouldBe(0, result);  // Expect -1 + 1 to equal 0

    result = add(-2, -3);
    expectShouldBe(-5, result); // Expect -2 + -3 to equal -5

    return true;  // Test passed
}

int main() {
    // Registering the test function with a description
    registerTest(testAddFunction, "Test for 'add' function");

    // Running all the registered tests
    runTests();

    return 0;
}
```

---

## ThreadPool
Thread pool to use threads simply in linux, just submit functions and arguments to do and it will be done.
The thread pool is a singleton. Only one thread pool exists and is allowed.

### Functions
| Function                 | Description                                      |
|--------------------------|--------------------------------------------------|
| `threadPoolInit`  | Creates the thread pool with the specified number of threads (0 for as many threads as cpu cores) |
| `threadPoolTaskPush`  | Add a function and argument to the queue which will be picked up by some thread |
| `threadPoolWaitToFinish`  | Blocking wait for all threads to be idle and all submitted tasks done |
| `threadPoolDestroy`  | Destroys the thread pool |

### Examples
```c
#include "threadPool.h"
#include <stdio.h>

// Example task function
void printMessage(void* message) {
    printf("%s\n", (char*)message);
}

int main() {

    // Create the thread pool with 2 threads
    if (!threadPoolInit(2)) {
        printf("Failed to create thread pool.\n");
        return -1;
    }

    // Add tasks to the pool
    const char* message = "Hello from thread!";
    for (int i = 0; i < 5; ++i)
    {
      threadPoolTaskPush(printMessage, (void*)message);
    }

    threadPoolWaitToFinish();
    
    threadPoolDestroy();

    return 0;
}
```


## Building and Linking

To use this library in your project:

1. Include the header files `logger.h`, `asserts.h`, and `filesystem.h`.
2. Makefile is included for compiling if needed
3. Here is the tree structure of a sample Project
```bash
├── defines.h
├── library
│   ├── bin
│   │   └── libForge.so
│   ├── include
│   │   ├── asserts.h
│   │   ├── expect.h
│   │   ├── filesystem.h
│   │   ├── hashMap.h
│   │   ├── linearAlloc.h
│   │   ├── logger.h
│   │   ├── objectPool.h
│   │   ├── orderedSet.h
│   │   ├── testManager.h
│   │   └── threadPool.h
│   └── Makefile
├── main.c

clang -o myapp main.c -I./library/include -L./library/bin -lAsher -Wl,-rpath=./library/bin
```

---

## License

The Forge Utilities Library is open source and available for use under the MIT License. See the LICENSE file for more details.
