#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

// Worker thread function
static void* worker_thread(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Wait for work or shutdown
        while (pool->task_queue_head == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        if (pool->shutdown && pool->task_queue_head == NULL) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // Dequeue task
        task_t* task = pool->task_queue_head;
        pool->task_queue_head = task->next;
        if (pool->task_queue_head == NULL) {
            pool->task_queue_tail = NULL;
        }
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // Execute task
        pthread_mutex_lock(&pool->active_mutex);
        pool->active_threads++;
        pthread_mutex_unlock(&pool->active_mutex);
        
        task->func(task->arg);
        free(task);
        
        pthread_mutex_lock(&pool->active_mutex);
        pool->active_threads--;
        pthread_cond_signal(&pool->idle_cond);
        pthread_mutex_unlock(&pool->active_mutex);
    }
    
    return NULL;
}

int thread_pool_init(thread_pool_t* pool, int thread_count) {
    pool->thread_count = thread_count;
    pool->task_queue_head = NULL;
    pool->task_queue_tail = NULL;
    pool->shutdown = false;
    pool->active_threads = 0;
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    pthread_mutex_init(&pool->active_mutex, NULL);
    pthread_cond_init(&pool->idle_cond, NULL);
    
    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    if (!pool->threads) {
        return -1;
    }
    
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            pool->shutdown = true;
            pthread_cond_broadcast(&pool->queue_cond);
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            free(pool->threads);
            return -1;
        }
    }
    
    return 0;
}

int thread_pool_submit(thread_pool_t* pool, task_func_t func, void* arg) {
    task_t* task = malloc(sizeof(task_t));
    if (!task) {
        return -1;
    }
    
    task->func = func;
    task->arg = arg;
    task->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_mutex);
        free(task);
        return -1;
    }
    
    if (pool->task_queue_tail == NULL) {
        pool->task_queue_head = task;
        pool->task_queue_tail = task;
    } else {
        pool->task_queue_tail->next = task;
        pool->task_queue_tail = task;
    }
    
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    return 0;
}

void thread_pool_wait(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->active_mutex);
    while (pool->active_threads > 0) {
        pthread_cond_wait(&pool->idle_cond, &pool->active_mutex);
    }
    pthread_mutex_unlock(&pool->active_mutex);
}

void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    free(pool->threads);
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    pthread_mutex_destroy(&pool->active_mutex);
    pthread_cond_destroy(&pool->idle_cond);
}

int thread_pool_get_active(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->active_mutex);
    int active = pool->active_threads;
    pthread_mutex_unlock(&pool->active_mutex);
    return active;
}
