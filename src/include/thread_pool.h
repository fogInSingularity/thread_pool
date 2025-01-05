#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#if defined (__STDC_NO_THREADS__)
#error "this thread_pool implementation is dependent on thread.h"
#endif // __STDC_NO_THREADS__

#if defined (__STDC_NO_ATOMICS__)
#error "this thread_pool implementation is dependent on stdatomic.h"
#endif // __STDC_NO_ATOMICS__

#include <stdlib.h>

typedef enum {
    ThreadPoolError_kOk            = 0,
    ThreadPoolError_kTaskQueueFull = 1,
} ThreadPoolError;

typedef struct ThreadPool ThreadPool;
typedef void (*TaskFunc)(void* task_arg);

ThreadPool* ThreadPoolCreate(size_t n_threads, size_t queue_size);
void ThreadPoolDestroy(ThreadPool* thread_pool);

ThreadPoolError ThreadPoolQueueTask(ThreadPool* thread_pool, TaskFunc task_func, void* task_arg);

#endif // THREAD_POOL_H_
