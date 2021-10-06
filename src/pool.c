#include <pool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void handle_client(int client);
void *_worker(void *args)
{
    task_t task;
    threadpool_t *pool = args;
    while (1)
    {
        pthread_mutex_lock(&pool->mutex);
        read(pool->command_pipe[0], &task, sizeof(task_t));
        pthread_mutex_unlock(&pool->mutex);
        if (task.del == NULL)
        {
            break;
        }
        else
        {
            void *ret = task.del(task.args);
            if (ret != NULL)
            {
                event_t event = ret;
                write(pool->event_pipe[1], &event, sizeof(event_t));
            }
        }
    }
    return NULL;
}

void pool_initialize(threadpool_t *pool, size_t thread_count)
{
    pipe(pool->command_pipe);
    pipe(pool->event_pipe);
    pthread_mutex_init(&pool->mutex, NULL);
    pool->threads = calloc(sizeof(threadpool_t), thread_count);
    for (size_t i = 0; i < thread_count; i++)
    {
        pthread_t thread;
        pthread_create(&thread, NULL, _worker, pool);
        pool->threads[i] = thread;
    }
    pool->thread_count = thread_count;
}

void pool_destroy(threadpool_t *pool)
{
    for (size_t i = 0; i < pool->thread_count; i++)
    {
        pool_execute(pool, NULL, NULL);
    }
    for (size_t i = 0; i < pool->thread_count; i++)
    {
        void *ret;
        pthread_join(pool->threads[i], &ret);
    }
    free(pool->threads);
    pthread_mutex_destroy(&pool->mutex);
    close(pool->command_pipe[0]);
    close(pool->command_pipe[1]);
    close(pool->event_pipe[0]);
    close(pool->event_pipe[1]);
}

void pool_execute(threadpool_t *pool, delegate_t del, void *args)
{
    task_t task;
    task.del = del;
    task.args = args;
    write(pool->command_pipe[1], &task, sizeof(task_t));
}

event_t pool_poll(threadpool_t *pool)
{
    event_t event;
    read(pool->event_pipe[0], &event, sizeof(event_t));
    return event;
}