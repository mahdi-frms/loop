#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <hashmap.h>
#include <server.h>
#include <loop-internal.h>

int poll_dual(int fd1, int fd2)
{
    struct pollfd fds[2];
    fds[0].fd = fd1;
    fds[0].events = POLLIN;
    fds[1].fd = fd2;
    fds[1].events = POLLIN;
    poll(fds, 2, -1);
    return fds[1].revents == POLLIN;
}

size_t poll_array(int *fds, size_t len)
{
    struct pollfd pfds[len];
    for (size_t i = 0; i < len; i++)
    {
        pfds[i].fd = fds[i];
        pfds[i].events = POLLIN;
    }
    poll(pfds, len, -1);
    for (size_t i = 0; i < len; i++)
    {
        if (pfds[i].revents == POLLIN)
        {
            return i;
        }
    }
    return 0;
}

void evloop_worker_readline(void *_, int *evl_pipe)
{
    char *line = NULL;
    size_t alloc;
    while (1)
    {
        if (poll_dual(evl_pipe[0], STDIN_FILENO) == 0)
        {
            break;
        }
        else
        {
            getline(&line, &alloc, stdin);
            message_t mes;
            message_readline *_mes = malloc(sizeof(message_readline));
            _mes->string = line;
            mes.ptr = _mes;
            mes.mtype = mtype_readline;
            write(evl_pipe[1], &mes, sizeof(message_t));
        }
    }
}

void evloop_worker_write_client(void *args, int *evl_pipe)
{
    arglist_write_client arglist = *(arglist_write_client *)args;
    write(arglist.client, arglist.message, strlen(arglist.message));
    message_t mes;
    message_terminate *_mes = malloc(sizeof(message_terminate));
    mes.ptr = _mes;
    mes.mtype = mtype_terminate;
    write(evl_pipe[1], &mes, sizeof(message_t));
    free(arglist.message);
    free(args);
}

void evloop_worker_accept_client(void *arg, int *evl_pipe)
{
    arglist_accept_client *arglist = arg;
    server_t *server = arglist->server;
    while (1)
    {
        if (poll_dual(evl_pipe[0], server->fd) == 0)
        {
            break;
        }
        else
        {
            message_t mes;
            message_accept_client *_mes = malloc(sizeof(message_accept_client));
            _mes->client = server_accept(server);
            mes.ptr = _mes;
            mes.mtype = mtype_accept_client;
            write(evl_pipe[1], &mes, sizeof(message_t));
        }
    }
}

void evloop_worker_read_client(void *arg, int *evl_pipe)
{
    arglist_read_client *arglist = arg;
    int client = arglist->client;
    while (1)
    {
        if (poll_dual(evl_pipe[0], client) == 0)
        {
            // event loop
            break;
        }
        else
        {
            // client message
            message_t mes;
            char *string = server_recieve(client);
            if (strcmp(string, "") == 0)
            {
                // client closed
                mes.mtype = mtype_terminate;
                message_terminate *_mes = malloc(sizeof(message_terminate));
                mes.ptr = _mes;
                write(evl_pipe[1], &mes, sizeof(message_t));
                break;
            }
            else
            {
                // client message
                mes.mtype = mtype_read_client;
                message_read_client *_mes = malloc(sizeof(message_read_client));
                _mes->client = client;
                _mes->string = string;
                mes.ptr = _mes;
                write(evl_pipe[1], &mes, sizeof(message_t));
            }
        }
    }
    close(client);
    free(arg);
}

void evloop_initialize(evloop_t *loop, size_t thread_count)
{
    loop->end = 0;
    evloop_task_hmap_init(loop);
    pool_initialize(&loop->pool, thread_count);
}

void evloop_destroy(evloop_t *loop)
{
    pool_destroy(&loop->pool);
    evloop_task_hmap_destory(loop);
}

int evloop_task_cmp(const void *t1, const void *t2, void *_)
{
    int p1 = ((evloop_task *)t1)->pipe[0];
    int p2 = ((evloop_task *)t2)->pipe[0];
    if (p1 > p2)
    {
        return 1;
    }
    else if (p1 < p2)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

uint64_t evloop_task_hash(const void *task, uint64_t seed0, uint64_t seed1)
{
    int p = ((evloop_task *)task)->pipe[0];
    return hashmap_sip(&p, sizeof(int), seed0, seed1);
}

bool evloop_task_iter_readpipe(const void *item, void *udata)
{
    int **array_ptr = udata;
    **array_ptr = ((evloop_task *)item)->pipe[0];
    (*array_ptr)++;
    return true;
}

bool evloop_task_iter_writepipe(const void *item, void *udata)
{
    int **array_ptr = udata;
    **array_ptr = ((evloop_task *)item)->pipe[1];
    (*array_ptr)++;
    return true;
}

void evloop_task_hmap_init(evloop_t *loop)
{
    loop->map = hashmap_new(sizeof(evloop_task), 0, 0, 0, evloop_task_hash, evloop_task_cmp, NULL, NULL);
}
void evloop_task_hmap_destory(evloop_t *loop)
{
    hashmap_free(loop->map);
}
void evloop_task_hmap_add(evloop_t *loop, evloop_task *task)
{
    hashmap_set(loop->map, task);
}
evloop_task *evloop_task_hmap_get(evloop_t *loop, int pipe_fd)
{
    evloop_task task;
    task.pipe[0] = pipe_fd;
    return hashmap_get(loop->map, &task);
}
void evloop_task_hmap_delete(evloop_t *loop, int pipe_fd)
{
    evloop_task task;
    task.pipe[0] = pipe_fd;
    hashmap_delete(loop->map, &task);
}

int *evloop_task_hmap_list_readpipe(evloop_t *loop, size_t *len)
{
    *len = hashmap_count(loop->map);
    int *array = calloc(sizeof(int), *len);
    hashmap_scan(loop->map, evloop_task_iter_readpipe, &array);
    return array - *len;
}

int *evloop_task_hmap_list_writepipe(evloop_t *loop, size_t *len)
{
    *len = hashmap_count(loop->map);
    int *array = calloc(sizeof(int), *len);
    hashmap_scan(loop->map, evloop_task_iter_writepipe, &array);
    return array - *len;
}

void evloop_do_readline(evloop_t *loop, callback_readline cb)
{
    arglist_readline *args = malloc(sizeof(arglist_readline));
    evloop_task *task = malloc(sizeof(evloop_task));
    task->cb = cb;
    pool_execute(&loop->pool, evloop_worker_readline, args, task->pipe);
    evloop_task_hmap_add(loop, task);
}

void evloop_do_accpet_client(evloop_t *loop, server_t *server, callback_accept_client cb)
{
    arglist_accept_client *args = malloc(sizeof(arglist_accept_client));
    args->server = server;
    evloop_task *task = malloc(sizeof(evloop_task));
    task->cb = cb;
    pool_execute(&loop->pool, evloop_worker_accept_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
}

void evloop_do_read_client(evloop_t *loop, int client, callback_read_client cb)
{
    arglist_read_client *args = malloc(sizeof(arglist_read_client));
    args->client = client;
    evloop_task *task = malloc(sizeof(evloop_task));
    task->cb = cb;
    pool_execute(&loop->pool, evloop_worker_read_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
}

void evloop_do_write_client(evloop_t *loop, int client, char *message)
{
    arglist_write_client *args = malloc(sizeof(arglist_write_client));
    args->client = client;
    args->message = message;
    evloop_task *task = malloc(sizeof(evloop_task));
    task->cb = NULL;
    pool_execute(&loop->pool, evloop_worker_write_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
}

int evloop_poll(evloop_t *loop)
{
    size_t len;
    int *pollfds = evloop_task_hmap_list_readpipe(loop, &len);
    return pollfds[poll_array(pollfds, len)];
}

message_t evloop_get_message(int pollfd)
{
    message_t mes;
    read(pollfd, &mes, sizeof(message_t));
    return mes;
}

void evloop_terminate(evloop_t *loop)
{
    loop->end = 1;
}

void evloop_abort(evloop_t *loop)
{
    char term = '\n';
    size_t len;
    int *pollfds = evloop_task_hmap_list_writepipe(loop, &len);
    for (size_t i = 0; i < len; i++)
    {
        write(pollfds[i], &term, sizeof(char));
    }
}

void evloop_main_loop(evloop_t *loop)
{
    while (loop->end == 0)
    {
        int pollfd = evloop_poll(loop);
        evloop_task *task = evloop_task_hmap_get(loop, pollfd);
        message_t mes = evloop_get_message(pollfd);
        if (mes.mtype == mtype_terminate)
        {
            evloop_task_hmap_delete(loop, pollfd);
        }
        else if (mes.mtype == mtype_readline)
        {
            message_readline *_mes = mes.ptr;
            callback_readline cb = task->cb;
            cb(loop, _mes->string);
        }
        else if (mes.mtype == mtype_accept_client)
        {
            message_accept_client *_mes = mes.ptr;
            callback_accept_client cb = task->cb;
            cb(loop, _mes->client);
        }
        else if (mes.mtype == mtype_read_client)
        {
            message_read_client *_mes = mes.ptr;
            callback_read_client cb = task->cb;
            cb(loop, _mes->client, _mes->string);
        }
        else
        {
            // err
            // undefined message
        }
        free(mes.ptr);
    }

    evloop_abort(loop);
}