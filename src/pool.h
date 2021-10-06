#ifndef POOL_H
#define POOL_H

#include <pthread.h>

struct threadpool_t
{
    pthread_t *threads;
    size_t thread_count;
    int queue_pipe[2];
    pthread_mutex_t mutex;
};
typedef struct threadpool_t threadpool_t;

typedef void (*delegate_t)(void *, int *);
struct event_t
{
    void *ret;
    size_t task_id;
};
typedef struct event_t event_t;

struct task_t
{
    delegate_t del;
    int pipe[2];
    void *args;
};
typedef struct task_t task_t;

void pool_initialize(threadpool_t *pool, size_t thread_count);
void pool_destroy(threadpool_t *pool);
void pool_execute(threadpool_t *pool, delegate_t del, void *args, int *pipe_fds);
event_t pool_poll(threadpool_t *pool);

#endif