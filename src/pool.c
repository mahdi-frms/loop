#include <pool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void handle_client(int client);
void *_worker(void *args)
{
    int client;
    threadpool_t *pool = args;
    while (1)
    {
        pthread_mutex_lock(&pool->mutex);
        read(pool->pipe_fd[0], &client, sizeof(int));
        pthread_mutex_unlock(&pool->mutex);
        if (client == 0)
        {
            break;
        }
        else
        {
            handle_client(client);
        }
    }
    return NULL;
}

void pool_initialize(threadpool_t *pool, size_t thread_count)
{
    pipe(pool->pipe_fd);
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

void pool_terminate(threadpool_t *pool)
{
    for (size_t i = 0; i < pool->thread_count; i++)
    {
        pool_execute(pool, 0);
    }
    for (size_t i = 0; i < pool->thread_count; i++)
    {
        void *ret;
        pthread_join(pool->threads[i], &ret);
    }
    free(pool->threads);
    pthread_mutex_destroy(&pool->mutex);
    close(pool->pipe_fd[0]);
    close(pool->pipe_fd[1]);
}

void pool_execute(threadpool_t *pool, int client)
{
    write(pool->pipe_fd[1], &client, sizeof(int));
}