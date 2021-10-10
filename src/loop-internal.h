#ifndef LOOP_INTERNAL_H
#define LOOP_INTERNAL_H

#include <loop.h>
#include <pool.h>
#include <hashmap.h>
#include <server.h>

typedef struct hashmap hashmap;

struct evloop_task
{
    uint64_t id;
    int pipe[2];
    void *cb;
};
typedef struct evloop_task evloop_task;

struct arglist_sock_write_client
{
    int client;
    char *message;
};
typedef struct arglist_sock_write_client arglist_sock_write_client;

struct arglist_sock_read_client
{
    int client;
};
typedef struct arglist_sock_read_client arglist_sock_read_client;

struct arglist_sock_accept_client
{
    server_t *server;
};
typedef struct arglist_sock_accept_client arglist_sock_accept_client;

struct arglist_sock_create_server
{
    int port;
};
typedef struct arglist_sock_create_server arglist_sock_create_server;

struct arglist_readline
{
};
typedef struct arglist_readline arglist_readline;

struct message_t
{
    mtype_t mtype;
    void *ptr;
};
typedef struct message_t message_t;

int poll_dual(int fd1, int fd2);
size_t poll_array(int *fds, size_t len);

evloop_task *evloop_task_create(evloop_t *loop, void *cb);
int evloop_task_cmp(const void *t1, const void *t2, void *_);
uint64_t evloop_task_hash(const void *task, uint_fast64_t seed0, uint_fast64_t seed1);
void evloop_task_hmap_init(evloop_t *loop);
void evloop_task_hmap_destory(evloop_t *loop);
void evloop_task_hmap_add(evloop_t *loop, evloop_task *task);
evloop_task *evloop_task_hmap_get(evloop_t *loop, int pipe_fd);
void evloop_task_hmap_delete(evloop_t *loop, int pipe_fd);
int *evloop_task_hmap_list_readpipe(evloop_t *loop, size_t *len);

// tasks
void evloop_worker_readline(void *_, int *evl_pipe);
void evloop_worker_sock_write_client(void *args, int *evl_pipe);
void evloop_worker_sock_accept_client(void *arg, int *evl_pipe);
void evloop_worker_sock_read_client(void *arg, int *evl_pipe);
void evloop_worker_sock_create_server(void *arg, int *evl_pipe);

#endif