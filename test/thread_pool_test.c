#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <threads.h>
#include <stdatomic.h>

#include "thread_pool.h"

// tasks:

void increment_task(void* arg) {
    atomic_int* value = (atomic_int*)arg;
    atomic_fetch_add(value, 1);
    // fprintf(stderr, "Task executed by thread: %lu\n", thrd_current());
}

void calculation_task(void* arg) {
    int* values = (int*)arg;
    values[2] = values[0] + values[1];
    // fprintf(stderr, "Calculation task executed by thread: %lu\n", thrd_current());
}

void sleeping_task(void* arg) {
    (void)arg;
    thrd_sleep(&(struct timespec){.tv_sec = 0, .tv_nsec = 100000000}, NULL); // 100 ms
    // fprintf(stderr, "Sleeping task executed by thread: %lu\n", thrd_current());
}

// tests:

void test_basic_task_execution() {
    fprintf(stderr, "Running basic task execution test...\n");
    int expected_counter = 10;

    ThreadPool* pool = ThreadPoolCreate(4, expected_counter + 1);

    atomic_int counter;
    atomic_init(&counter, 0);
    for (int i = 0; i < expected_counter; ++i) {
        assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    }

    // thrd_sleep(&(struct timespec){.tv_sec = 1, .tv_nsec = 0}, NULL);

    ThreadPoolDestroy(pool);
    assert(counter == expected_counter);
    fprintf(stderr, "Basic task execution test passed!\n");
}

void test_queue_full() {
    fprintf(stderr, "Running queue full test...\n");
    ThreadPool* pool = ThreadPoolCreate(1, 2);

    atomic_int counter;
    atomic_init(&counter, 0);

    assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kTaskQueueFull);

    ThreadPoolDestroy(pool);
    fprintf(stderr, "Queue full test passed!\n");
}

void test_multiple_task_types() {
    fprintf(stderr, "Running multiple task types test...\n");
    ThreadPool* pool = ThreadPoolCreate(2, 5);

    int values[3] = {5, 10, 0};
    assert(ThreadPoolQueueTask(pool, calculation_task, values) == ThreadPoolError_kOk);

    atomic_int counter;
    atomic_init(&counter, 0);
    assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    thrd_sleep(&(struct timespec){.tv_sec = 1, .tv_nsec = 0}, NULL);

    ThreadPoolDestroy(pool);
    assert(values[2] == 15);
    assert(counter == 2);
    fprintf(stderr, "Multiple task types test passed!\n");
}

void test_many_tasks() {
    fprintf(stderr, "Running many tasks test...\n");
    const int num_tasks = 10000;
    ThreadPool* pool = ThreadPoolCreate(8, num_tasks);

    atomic_int counter;
    atomic_init(&counter, 0);
    for (int i = 0; i < num_tasks; ++i) {
        assert(ThreadPoolQueueTask(pool, increment_task, &counter) == ThreadPoolError_kOk);
    }
    thrd_sleep(&(struct timespec){.tv_sec = 2, .tv_nsec = 0}, NULL);

    ThreadPoolDestroy(pool);
    assert(counter == num_tasks);
    fprintf(stderr, "Many tasks test passed!\n");
}

void test_sleeping_tasks(){
    fprintf(stderr, "Running sleeping tasks test...\n");
    const int num_tasks = 10;
    ThreadPool* pool = ThreadPoolCreate(4, num_tasks);

    for (int i = 0; i < num_tasks; ++i) {
        assert(ThreadPoolQueueTask(pool, sleeping_task, NULL) == ThreadPoolError_kOk);
    }

    thrd_sleep(&(struct timespec){.tv_sec = 2, .tv_nsec = 0}, NULL);

    ThreadPoolDestroy(pool);
    fprintf(stderr, "Sleeping tasks test passed!\n");
}

int main() {
    test_basic_task_execution();
    // test_queue_full(); // FIXME fix test
    test_multiple_task_types();
    test_many_tasks();
    test_sleeping_tasks();

    fprintf(stderr, "All tests finished.\n");
    return 0;
}
