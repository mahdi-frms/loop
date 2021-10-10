#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <server.h>
#include <loop.h>

#define POOL_ACCEPT 1
#define POOL_READLINE 2

char *reverse(const char *string)
{
    size_t size = strlen(string);
    char *rev = malloc(size + 1);
    for (size_t i = 0; i < size - 1; i++)
    {
        rev[i] = string[size - 2 - i];
    }
    rev[size - 1] = '\n';
    rev[size] = '\0';
    return rev;
}

void on_line(evloop_t *loop, char *line)
{
    // printf("task id = %lu\n", evloop_task_id(loop));
    if (strcmp(line, "fin\n") == 0)
    {
        evloop_terminate(loop);
    }
    else
    {
        printf("line=%s", line);
    }
}

void on_message(evloop_t *loop, int client, char *message)
{
    // printf("task id = %lu\n", evloop_task_id(loop));
    char *rev = reverse(message);
    printf("client: %s", message);
    free(message);
    evloop_do_sock_write_client(loop, client, rev);
}

void on_client(evloop_t *loop, int client)
{
    // printf("task id = %lu\n", evloop_task_id(loop));
    printf("new client\n");
    evloop_do_sock_read_client(loop, client, on_message);
}

void on_server(evloop_t *loop, server_t server)
{
    server_t *_server = malloc(sizeof(server_t));
    *_server = server;
    evloop_do_sock_accpet_client(loop, _server, on_client);
    printf("listening on port %d...\n", _server->port);
}

int main(int argc, char **argv)
{
    int port = 8080;
    int threads = 8;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }
    if (argc > 2)
    {
        threads = atoi(argv[2]);
    }

    evloop_t loop;
    evloop_initialize(&loop, threads);
    evloop_do_readline(&loop, on_line);
    evloop_do_sock_create_server(&loop, port, on_server);
    evloop_main_loop(&loop);
    evloop_destroy(&loop);
}