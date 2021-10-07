#include <pool.h>
#include <hashmap.h>
#include <server.h>

typedef struct hashmap hashmap;

enum mtype_t
{
    mtype_terminate,
    mtype_readline,
    mtype_accept_client,
    mtype_close_client,
    mtype_get_message
};

typedef enum mtype_t mtype_t;

struct evloop_task
{
    int pipe[2];
    void *cb;
};
typedef struct evloop_task evloop_task;

struct evloop_t
{
    threadpool_t pool;
    hashmap *map;
};
typedef struct evloop_t evloop_t;

struct message_t
{
    mtype_t mtype;
    void *ptr;
};
typedef struct message_t message_t;

typedef void (*callback_readline)(char *);
typedef void (*callback_accept_client)(int client);

void evloop_initialize(evloop_t *loop, size_t thread_count);
void evloop_main_loop(evloop_t *loop);
int evloop_poll(evloop_t *loop);
void evloop_destroy(evloop_t *loop);

int evloop_task_cmp(const void *t1, const void *t2, void *_);
uint64_t evloop_task_hash(const void *task, uint_fast64_t seed0, uint_fast64_t seed1);
void evloop_task_hmap_init(evloop_t *loop);
void evloop_task_hmap_destory(evloop_t *loop);
void evloop_task_hmap_add(evloop_t *loop, evloop_task *task);
evloop_task *evloop_task_hmap_get(evloop_t *loop, int pipe_fd);
void evloop_task_hmap_delete(evloop_t *loop, int pipe_fd);
int *evloop_task_hmap_list(evloop_t *loop, size_t *len);

// api
void evloop_do_readline(evloop_t *loop, callback_readline cb);
void evloop_do_accpet_client(evloop_t *loop, server_t *server, callback_accept_client cb);

// tasks
void evloop_worker_readline(void *_, int *evl_pipe);
void evloop_worker_accept_client(void *arg, int *evl_pipe);