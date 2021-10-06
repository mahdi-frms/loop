#ifndef POOL_H
#define POOL_H

#include <pthread.h>

struct threadpool_t
{
    pthread_t *threads;
    size_t thread_count;
    int pipe_fd[2];
    pthread_mutex_t mutex;
};
typedef struct threadpool_t threadpool_t;

typedef void (*delegate_t)(void *);

struct task_t
{
    delegate_t del;
    void *args;
};
typedef struct task_t task_t;

void pool_initialize(threadpool_t *pool, size_t thread_count);
void pool_terminate(threadpool_t *pool);
void pool_execute(threadpool_t *pool, delegate_t del, void *args);

#endif