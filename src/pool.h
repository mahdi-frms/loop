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
void pool_initialize(threadpool_t *pool, size_t thread_count);
void pool_terminate(threadpool_t *pool);
void pool_execute(threadpool_t *pool, int client);

#endif