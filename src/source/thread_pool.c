#include "thread_pool.h"

#include <threads.h>
#include <stdatomic.h>

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ThreadPoolTask {
    TaskFunc task_func;
    void* task_arg;
} Task;

struct ThreadPool {
    mtx_t pool_mutex;
    cnd_t pool_cond;

    thrd_t* pool;
    size_t pool_size;

    atomic_ullong waiting_threads;
    atomic_bool should_destroy;

    struct {
        Task* task_buf;
        size_t head;
        size_t tail;
        atomic_ullong waiting_task_cnt;
    } task_ring;
    size_t ring_size;
};

// static -----------------------------------------------------------------------------------------

int Worker(void* worker_arg);

// wrapper for calloc that aborts if cant alloc
void* ThreadAlloc(size_t nmems, size_t size);
void ThreadFree(void* ptr);

// global -----------------------------------------------------------------------------------------

ThreadPool* ThreadPoolCreate(size_t n_threads, size_t queue_size) {
    ThreadPool* thread_pool = ThreadAlloc(1, sizeof(ThreadPool));
    thread_pool->pool = ThreadAlloc(n_threads, sizeof(thrd_t));
    thread_pool->task_ring.task_buf = ThreadAlloc(queue_size, sizeof(Task));

    mtx_init(&thread_pool->pool_mutex, mtx_plain);
    cnd_init(&thread_pool->pool_cond);
    atomic_init(&thread_pool->waiting_threads, 0);
    atomic_init(&thread_pool->should_destroy, false);

    thread_pool->task_ring.head = 0;
    thread_pool->task_ring.tail = 0;
    atomic_init(&thread_pool->task_ring.waiting_task_cnt, 0);
    thread_pool->ring_size = queue_size;
    
    thread_pool->pool_size = n_threads;
    for (size_t thread_i = 0; thread_i < n_threads; thread_i++) {
        thrd_create(&thread_pool->pool[thread_i], Worker, thread_pool);
    }
    
    while (atomic_load(&thread_pool->waiting_threads) < n_threads) {}

    return thread_pool;
}

void ThreadPoolDestroy(ThreadPool* thread_pool) {
    assert(thread_pool != NULL);

    // while (thread_pool->task_ring.head != thread_pool->task_ring.tail) {}
    // while (atomic_load(&thread_pool->task_ring.waiting_task_cnt) != 0) {}
    
    atomic_store(&thread_pool->should_destroy, true);
    cnd_broadcast(&thread_pool->pool_cond);
    while(atomic_load(&thread_pool->waiting_threads) != 0) {}

    for (size_t thread_i = 0; thread_i < thread_pool->pool_size; thread_i++) {
        int thread_exit_res = 0;
        thrd_join(thread_pool->pool[thread_i], &thread_exit_res);
    }

    mtx_destroy(&thread_pool->pool_mutex);
    cnd_destroy(&thread_pool->pool_cond);
    ThreadFree(thread_pool->task_ring.task_buf);
    ThreadFree(thread_pool->pool);
    ThreadFree(thread_pool);
}

ThreadPoolError ThreadPoolQueueTask(ThreadPool* thread_pool, TaskFunc task_func, void* task_arg) {
    assert(thread_pool != NULL);

    Task task = {
        .task_func = task_func,
        .task_arg = task_arg
    };

    mtx_lock(&thread_pool->pool_mutex);
    
    bool is_ring_full = atomic_load(&thread_pool->task_ring.waiting_task_cnt) 
                        == thread_pool->ring_size;

    ThreadPoolError err = (is_ring_full) 
                                ? ThreadPoolError_kTaskQueueFull
                                : ThreadPoolError_kOk;

    if (!is_ring_full) {
        thread_pool->task_ring.task_buf[thread_pool->task_ring.head] = task;
        thread_pool->task_ring.head = (thread_pool->task_ring.head + 1) 
                                      % thread_pool->ring_size;
        atomic_fetch_add(&thread_pool->task_ring.waiting_task_cnt, 1);

        cnd_signal(&thread_pool->pool_cond);
    }

    mtx_unlock(&thread_pool->pool_mutex);

    return err;
}

// static -----------------------------------------------------------------------------------------

int Worker(void* worker_arg) {
    assert(worker_arg != NULL);

    ThreadPool* thread_pool = (ThreadPool*)worker_arg;

    atomic_fetch_add(&thread_pool->waiting_threads, 1);

    // while (!atomic_load(&thread_pool->should_destroy)) {
    while (true) {
        mtx_lock(&thread_pool->pool_mutex);
           
        bool should_break = false;

        while (atomic_load(&thread_pool->task_ring.waiting_task_cnt) == 0) {
            if (atomic_load(&thread_pool->should_destroy)) {
                mtx_unlock(&thread_pool->pool_mutex);
                should_break = true;
                break;
            }

            cnd_wait(&thread_pool->pool_cond, &thread_pool->pool_mutex);
        }

        if (should_break) { break; }

        Task task = thread_pool->task_ring.task_buf[thread_pool->task_ring.tail];
        memset(&thread_pool->task_ring.task_buf[thread_pool->task_ring.tail], 0, sizeof(Task));

        thread_pool->task_ring.tail = (thread_pool->task_ring.tail + 1) % thread_pool->ring_size;

        atomic_fetch_sub(&thread_pool->task_ring.waiting_task_cnt, 1);

        mtx_unlock(&thread_pool->pool_mutex);

        // execute task
        task.task_func(task.task_arg);
    }

    atomic_fetch_sub(&thread_pool->waiting_threads, 1);

    return 0;
}

void* ThreadAlloc(size_t nmems, size_t size) {
    void* ptr = calloc(nmems, size);
    if (ptr == NULL) {
        fprintf(stderr, "Thread pool fatal error, cant allocate memory: %s\n", strerror(errno));

        abort();
    }

    return ptr;
}

void ThreadFree(void* ptr) {
    free(ptr);
}
