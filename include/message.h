#ifndef MESSAGE_H
#define MESSAGE_H

struct evloop_t;
typedef struct evloop_t evloop_t;

enum mtype_t
{
    mtype_terminate,
    mtype_readline,
    mtype_accept_client,
    mtype_read_client
};
typedef enum mtype_t mtype_t;

struct message_terminate
{
};
typedef struct message_terminate message_terminate;

struct message_readline
{
    char *string;
};
typedef struct message_readline message_readline;
typedef void (*callback_readline)(evloop_t *, char *);

struct message_read_client
{
    int client;
    char *string;
};
typedef struct message_read_client message_read_client;
typedef void (*callback_read_client)(evloop_t *, int, char *);

struct message_accept_client
{
    int client;
};
typedef struct message_accept_client message_accept_client;
typedef void (*callback_accept_client)(evloop_t *, int);

#endif