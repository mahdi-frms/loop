#include <stdio.h>
#include <unistd.h>
#include <loop-internal.h>
#include <stdlib.h>
#include <string.h>

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

uint64_t evloop_do_readline(evloop_t *loop, callback_readline cb)
{
    arglist_readline *args = malloc(sizeof(arglist_readline));
    evloop_task *task = evloop_task_create(loop, cb);
    pool_execute(&loop->pool, evloop_worker_readline, args, task->pipe);
    evloop_task_hmap_add(loop, task);
    return task->id;
}

uint64_t evloop_do_accpet_client(evloop_t *loop, server_t *server, callback_accept_client cb)
{
    arglist_accept_client *args = malloc(sizeof(arglist_accept_client));
    args->server = server;
    evloop_task *task = evloop_task_create(loop, cb);
    pool_execute(&loop->pool, evloop_worker_accept_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
    return task->id;
}

uint64_t evloop_do_read_client(evloop_t *loop, int client, callback_read_client cb)
{
    arglist_read_client *args = malloc(sizeof(arglist_read_client));
    args->client = client;
    evloop_task *task = evloop_task_create(loop, cb);
    pool_execute(&loop->pool, evloop_worker_read_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
    return task->id;
}

uint64_t evloop_do_write_client(evloop_t *loop, int client, char *message)
{
    arglist_write_client *args = malloc(sizeof(arglist_write_client));
    args->client = client;
    args->message = message;
    evloop_task *task = evloop_task_create(loop, NULL);
    pool_execute(&loop->pool, evloop_worker_write_client, args, task->pipe);
    evloop_task_hmap_add(loop, task);
    return task->id;
}