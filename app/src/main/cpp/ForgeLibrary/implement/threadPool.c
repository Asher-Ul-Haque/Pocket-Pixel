#include "../include/threadPool.h"
#include "../include/asserts.h"
#include "../include/logger.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>


// - - - Prototypes - - - 

static ThreadPool     POOL;
static volatile bool  created = false;
static volatile bool  running = false;


// - - -  Semaphore - - - 

static void seminit(Semaphore* SEM, u8 VALUE)
{
  FORGE_ASSERT_MESSAGE(SEM, "Cannot initialize a NULL  Semaphore");

  pthread_mutex_init(&SEM->lock,        NULL);
  pthread_cond_init (&SEM->conditional, NULL);
  SEM->value = VALUE;
}

static void semDestroy(Semaphore* SEM)
{
  FORGE_ASSERT_MESSAGE(SEM, "Cannot destroy a NULL  Semaphore");

  pthread_mutex_destroy(&SEM->lock);
  pthread_cond_destroy (&SEM->conditional);
}

static void semWait(Semaphore* SEM)
{
  FORGE_ASSERT_MESSAGE(SEM, "Cannot destroy a NULL  Semaphore");
  
  pthread_mutex_lock(&SEM->lock);
  while (SEM->value == 0) pthread_cond_wait(&SEM->conditional, &SEM->lock);
  SEM->value--;
  pthread_mutex_unlock(&SEM->lock);
}

static void semPost(Semaphore* SEM)
{
  FORGE_ASSERT_MESSAGE(SEM, "Cannot destroy a NULL  Semaphore");
  
  pthread_mutex_lock(&SEM->lock);
  SEM->value++;
  pthread_cond_signal(&SEM->conditional);
  pthread_mutex_unlock(&SEM->lock);
}


// - - - Task Queue - - - 

static bool taskQueueInit()
{
  TaskQueue* queue = &POOL.taskQueue;
  if (pthread_mutex_init(&queue->readWriteLock, NULL) != 0)
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to create read write lock for the queue");
    return false;
  };    
  seminit(&queue->availability, 0);

  queue->front  = NULL;
  queue->end    = NULL;
  queue->size   = 0;

  return true;
}

static void taskQueueDestroy()
{
  FORGE_ASSERT_MESSAGE(created, "Thread pool needs to be started first")

  TaskQueue* queue = &POOL.taskQueue;

  pthread_mutex_lock(&queue->readWriteLock);
  Task* current = queue->front;

  while (current)
  {
    Task* tmp = current;
    current   = current->previous;
    free(tmp);
  }

  pthread_mutex_unlock  (&queue->readWriteLock);
  pthread_mutex_destroy (&queue->readWriteLock);
  semDestroy            (&queue->availability);
}

static void taskQueuePush(Task* TASK)
{
  FORGE_ASSERT_MESSAGE(created, "Thread pool needs to be started first")

  TaskQueue* queue  = &POOL.taskQueue;
  TASK->previous    = NULL;

  pthread_mutex_lock(&queue->readWriteLock);
  if (queue->size == 0)
  {
    queue->front  = TASK;
    queue->end    = TASK;
  }
  else 
  {
    queue->end->previous  = TASK;
    queue->end            = TASK;
  }

  queue->size++;
  pthread_mutex_unlock(&queue->readWriteLock);

  semPost(&queue->availability);
}

static Task* taskQueuePull()
{
  FORGE_ASSERT_MESSAGE(created, "Thread pool needs to be started first")

  TaskQueue* queue  = &POOL.taskQueue;

  pthread_mutex_lock(&queue->readWriteLock);
  Task*      task   = queue->front;

  if (queue->size > 0)
  {
    queue->front = task->previous;
    if (queue->front == NULL) queue->end = NULL;

    queue->size--;
  }

  pthread_mutex_unlock(&queue->readWriteLock);
  return task;
}

static bool isTaskQueueEmpty()
{
  FORGE_ASSERT_MESSAGE(created, "Thread pool needs to be started first");
  
  pthread_mutex_lock(&POOL.taskQueue.readWriteLock);
  bool isEmpty = (POOL.taskQueue.size == 0);
  pthread_mutex_unlock(&POOL.taskQueue.readWriteLock);

  return isEmpty;
}


// - - - Threads - - - 

static void* threadDo(void* ARG)
{
  Thread* self = (Thread*)ARG;

  while (true)
  {
    semWait(&POOL.taskQueue.availability);
    if (!running) break;

    Task* task = taskQueuePull();

    if (task)
    {
      pthread_mutex_lock(&POOL.threadCountLock);
      POOL.numThreadsWorking++;
      pthread_mutex_unlock(&POOL.threadCountLock);

      // - - - run the task
      task->function(task->argument);
      free(task);

      bool noneLeft = isTaskQueueEmpty();

      pthread_mutex_lock(&POOL.threadCountLock);
      POOL.numThreadsWorking--;
      if (POOL.numThreadsWorking == 0 && noneLeft)
      {
        pthread_cond_broadcast(&POOL.allIdle);
      }
      pthread_mutex_unlock(&POOL.threadCountLock);
    }
  }

  return NULL;
}

static bool threadInit(i32 ID)
{
  Thread* thread  = (Thread*)malloc(sizeof(Thread));
  thread->id      = ID;

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  if (pthread_create(&thread->pthread, &attr, threadDo, (void*)thread) != 0)
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to initialize thread %d", ID);
    pthread_attr_destroy(&attr);
    free(thread);
    return false;
  }
  pthread_attr_destroy(&attr);

  pthread_mutex_lock(&POOL.threadCountLock);
  POOL.threads[ID] = thread;
  POOL.numThreadsAlive++;
  pthread_mutex_unlock(&POOL.threadCountLock);

  return true;
}

static void threadDestroy(i32 ID)
{
  FORGE_ASSERT_MESSAGE(created, "Thread pool needs to be started first")

  Thread* thread = POOL.threads[ID];
  pthread_join(thread->pthread, NULL);
  free(thread);
}


// - - - Thread pool - - - 

bool threadPoolInit(u8 THREAD_COUNT)
{
  FORGE_ASSERT_MESSAGE(!created, "Cannot simply recreate the thread pool. Call threadPoolDestroy() first!");
 
  // - - - assign the number of threads and pinning to cpu 
  POOL.numThreadsAlive     = 0;
  POOL.numThreadsWorking   = 0;
  u8 threadCount           = THREAD_COUNT;

  if (THREAD_COUNT == 0)   
  {
    i64 cores       = sysconf(_SC_NPROCESSORS_ONLN);
        threadCount = (cores > 0 && cores < 256) ? (u8)cores : 4;
    FORGE_LOG_WARNING("[THREAD POOL] : Creating a thread pool with hardware defined thread count: %d", threadCount);
  }

  FORGE_LOG_TRACE("[THREAD_POOL] : Starting with %d threads", threadCount);

  // - - - initialize the lock and conditional
  pthread_mutex_init(&(POOL.threadCountLock), NULL);
  pthread_cond_init(&POOL.allIdle, NULL);

  // - - - initialize task queue 
  if (!taskQueueInit())
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to initialize task queue");
    return false;    
  };

  // - - - make threads 
  POOL.threads = (Thread**) calloc (threadCount, sizeof(Thread*));
  if (POOL.threads == NULL)
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Failed to allocate memory for threads");
    return false;
  }
  for (u8 i = 0; i < threadCount; i++)    threadInit(i); 

  FORGE_LOG_TRACE("[THREAD POOL] : Waiting for all the threads to be ready")
  while (POOL.numThreadsAlive != threadCount){}
  FORGE_LOG_TRACE("[THREAD POOL] : Wait finished. All the threads are ready to go");

  created = true;
  running = true;
  return true;
}

void threadPoolTaskPush(void (*FUNCTION)(void*), void* ARGUMENT)
{
  FORGE_ASSERT_MESSAGE(created,  "Thread Pool is not started yet");
  FORGE_ASSERT_MESSAGE(FUNCTION, "Cannot add a NULL Function to a task");

  Task* newTask = (Task*)malloc(sizeof(Task));
  if (newTask == NULL)
  {
    FORGE_LOG_ERROR("[THREAD POOL] : Could not allocate memory for new job");
    return;
  }

  newTask->function = FUNCTION;
  newTask->argument = ARGUMENT;
  newTask->previous = NULL;

  taskQueuePush(newTask);
}

void threadPoolWaitToFinish()
{
  FORGE_ASSERT_MESSAGE(created, "Thread Pool is not started. Call `threadPoolCreate` first");

  FORGE_LOG_INFO("[THREAD POOL] : Waiting for all tasks to finish");
  pthread_mutex_lock(&POOL.threadCountLock);  
  while (POOL.taskQueue.size > 0 || !isTaskQueueEmpty())
  {
    FORGE_LOG_TRACE("[THREAD POOL] : Main thread waiting for all idle, current task queue size : %d, current threads working : %d", POOL.taskQueue.size, POOL.numThreadsWorking)
    pthread_cond_wait(&POOL.allIdle, &POOL.threadCountLock);
  }
  pthread_mutex_unlock(&POOL.threadCountLock);
  FORGE_LOG_INFO("[THREAD POOL] : Finished waiting for all tasks")
}

void threadPoolDestroy()
{
  FORGE_ASSERT_MESSAGE(created, "Thread Pool is not started. Call `threadPoolCreate` first");
  
  FORGE_LOG_WARNING("[THREAD POOL] : Destroying Thread Pool");

  // - - - signal all threads to stop
  running = false;

  // - - - wake up all threads 
  for (u8 i = 0; i < POOL.numThreadsAlive; ++i) semPost(&POOL.taskQueue.availability);
  for (u8 i = 0; i < POOL.numThreadsAlive; ++i) threadDestroy(i);

  free(POOL.threads);
  taskQueueDestroy();

  pthread_mutex_destroy(&POOL.threadCountLock);
  pthread_cond_destroy(&POOL.allIdle);
  
  created = false;
}
