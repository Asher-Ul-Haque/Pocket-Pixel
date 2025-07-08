#pragma once 
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>


typedef pthread_mutex_t Lock;
typedef pthread_cond_t  Conditional;

typedef struct Thread 
{
  i32                 id;       // - - - id of the thread
  pthread_t           pthread;  // - - - the actual thread
} Thread;

typedef struct Semaphore
{
  Lock        lock;
  Conditional conditional;
  u8          value;
} Semaphore;

typedef struct Task 
{
  void(*function)       (void*);        // - - - thread function 
  void*                 argument;       // - - - argument
  struct Task*          previous;       // - - - previous job
} Task;

typedef struct TaskQueue
{
  Lock                  readWriteLock;  // - - - used for queue r/w access
  Task*                 front;          // - - - pointer to front of the queue
  Task*                 end;            // - - - pointer to rear of the queue
  Semaphore       availability;   // - - - flag 
  u64                   size;           // - - - number of jobs in queue
} TaskQueue;

typedef struct ThreadPool 
{
  Lock          threadCountLock;        // - - - used for thread count update
  Thread**      threads;                // - - - the threads 
  volatile u8   numThreadsAlive;        // - - - how many threads are alive 
  volatile u8   numThreadsWorking;      // - - - threads currently working 
  Conditional   allIdle;                // - - - signal to wait 
  TaskQueue     taskQueue;              // - - - the tasks
} ThreadPool;


// - - - Functions - - - 

// - - - pass 0 as the number of threads to match CPU hardware specification
FORGE_API bool  threadPoolInit         (u8 THREAD_COUNT);

// - - - add all tasks to thread pool
FORGE_API void  threadPoolTaskPush     (void (*FUNCTION)(void*), void* ARGUMENT);

// - - - wait for all running tasks to finish and clear all queued tasks and free all memory
FORGE_API void  threadPoolDestroy      ();

// - - - blocking wait for all tasks to finish
FORGE_API void  threadPoolWaitToFinish ();

#ifdef __cplusplus
}
#endif
