#ifndef LOOP_H
#define LOOP_H

#include <pool.h>
#include <server.h>
#include <message.h>

struct evloop_t
{
    threadpool_t pool;
    void *map;
    int end;
};
typedef struct evloop_t evloop_t;

void evloop_initialize(evloop_t *loop, size_t thread_count);
void evloop_main_loop(evloop_t *loop);
int evloop_poll(evloop_t *loop);
void evloop_destroy(evloop_t *loop);
void evloop_terminate(evloop_t *loop);

// api
void evloop_do_readline(evloop_t *loop, callback_readline cb);
void evloop_do_accpet_client(evloop_t *loop, server_t *server, callback_accept_client cb);
void evloop_do_read_client(evloop_t *loop, int client, callback_read_client cb);
void evloop_do_write_client(evloop_t *loop, int client, char *message);

#endif