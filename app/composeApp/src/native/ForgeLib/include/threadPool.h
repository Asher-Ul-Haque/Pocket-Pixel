#pragma once 
#ifdef __linux__
#include "../../defines.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>


typedef pthread_t       Thread;
typedef pthread_mutex_t Lock;
typedef pthread_cond_t  Conditional;

struct Task;

typedef struct Task 
{
  void(*function)       (void*);        // - - - thread function 
  void*                 argument;       // - - - argument
} Task;

typedef struct TaskQueue
{
  Task*                 tasks;          // - - - array of tasks 
  int                   front;          // - - - queue variables
  int                   rear;
  int                   count;
  int                   capacity;
  Lock                  readLock;       // - - - lock for queue safety
  Conditional           availability;   // - - - conditional for tasks
} TaskQueue;

typedef struct ThreadPool 
{
  Thread*               threads;        // - - - worker threads 
  TaskQueue             queue;          // - - - task queue for the pool 
  int                   threadCount;    // - - - max threads 
  bool                  stop;           // - - - flag to stop the pool
} ThreadPool;

FORGE_API bool          createThreadPool    (ThreadPool* POOL,      int THREAD_COUNT,           int TASK_CAPACITY);
FORGE_API bool          destroyThreadPool   (ThreadPool* POOL);
FORGE_API void          pushTask            (ThreadPool* POOL,      void (*FUNCTION)(void*),    void* ARGUMENT);

#ifdef __cplusplus
}
#endif
#endif
