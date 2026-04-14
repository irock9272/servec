#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>
#include "config.h"

// Task function type
typedef void (*task_func_t)(void* arg);

// Task structure
typedef struct task {
    task_func_t func;
    void* arg;
    struct task* next;
} task_t;

// Thread pool structure
typedef struct {
    pthread_t* threads;
    int thread_count;
    task_t* task_queue_head;
    task_t* task_queue_tail;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    bool shutdown;
    int active_threads;
    pthread_mutex_t active_mutex;
    pthread_cond_t idle_cond;
} thread_pool_t;

// Initialize thread pool
int thread_pool_init(thread_pool_t* pool, int thread_count);

// Submit a task to the pool
int thread_pool_submit(thread_pool_t* pool, task_func_t func, void* arg);

// Wait for all tasks to complete
void thread_pool_wait(thread_pool_t* pool);

// Destroy thread pool
void thread_pool_destroy(thread_pool_t* pool);

// Get number of active threads
int thread_pool_get_active(thread_pool_t* pool);

#endif // THREAD_POOL_H
