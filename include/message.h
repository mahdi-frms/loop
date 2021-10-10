#ifndef MESSAGE_H
#define MESSAGE_H

struct evloop_t;
typedef struct evloop_t evloop_t;

enum mtype_t
{
    mtype_terminate,
    mtype_readline,
    mtype_sock_accept_client,
    mtype_sock_read_client,
    mtype_sock_create_server
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

struct message_sock_read_client
{
    int client;
    char *string;
};
typedef struct message_sock_read_client message_sock_read_client;
typedef void (*callback_sock_read_client)(evloop_t *, int, char *);

struct message_sock_accept_client
{
    int client;
};
typedef struct message_sock_accept_client message_sock_accept_client;
typedef void (*callback_sock_accept_client)(evloop_t *, int);

struct message_sock_create_server
{
    server_t server;
};
typedef struct message_sock_create_server message_sock_create_server;
typedef void (*callback_sock_create_server)(evloop_t *, server_t);

#endif