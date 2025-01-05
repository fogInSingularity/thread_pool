# thread_pool
my thread_pool library writen in c

## Installation:

### Prerequisites:
1. C compiler that supports C11 and thread.h + stdatomic.h
2. CMake (version 3.10 or later)
3. git

Clone repository:
```bash
git clone https://github.com/fogInSingularity/thread_pool.git
```

Navigate to the project directory:
```bash
cd thread_pool
```

Configure and build project:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel `nproc`
```

If you want to run tests:
Configure and build project:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build --parallel `nproc`
```

## Usage(API):

```c
typedef enum {
    ThreadPoolError_kOk            = 0,
    ThreadPoolError_kTaskQueueFull = 1,
} ThreadPoolError;

typedef struct ThreadPool ThreadPool;
typedef void (*TaskFunc)(void* task_arg);

ThreadPool* ThreadPoolCreate(size_t n_threads, size_t queue_size);
void ThreadPoolDestroy(ThreadPool* thread_pool);

ThreadPoolError ThreadPoolQueueTask(ThreadPool* thread_pool, TaskFunc task_func, void* task_arg);

```
