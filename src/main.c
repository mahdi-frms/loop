#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pool.h>
#include <server.h>

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

void handle_client(void *arg, emittor_t *_)
{
    int client = *(int *)arg;
    while (1)
    {
        char *string = server_recieve(client);
        if (string == NULL || strcmp(string, "") == 0 || strcmp(string, "\n") == 0)
        {
            break;
        }
        string = reverse(string);
        if (server_send(client, string, strlen(string)) == -1)
        {
            break;
        }
    }
    close(client);
    free(arg);
}

void accept_client(void *arg, emittor_t *emittor)
{
    server_t *server = arg;
    int *client;
    client = malloc(sizeof(int));
    *client = server_accept(server);
    if (*client == -1)
    {
        fprintf(stderr, "Error: server failed to establish connection\n");
        exit(1);
    }
    emittor_emit(emittor, client);
}

void get_line(void *_, emittor_t *emittor)
{
    char *line = NULL;
    size_t alloc;
    while (1)
    {
        getline(&line, &alloc, stdin);
        emittor_emit(emittor, line);
    }
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
    server_t server = server_create(port);
    threadpool_t pool;
    pool_initialize(&pool, threads);
    if (server.fd == -1)
    {
        perror("Error: server initialization failed\n");
        return 1;
    }
    if (server_listen(&server) == -1)
    {
        perror("Error: server failed to listen\n");
        return 1;
    }
    printf("listening on port %d...\n", port);
    pool_execute(&pool, accept_client, &server, POOL_ACCEPT);
    pool_execute(&pool, get_line, NULL, POOL_READLINE);
    while (1)
    {
        event_t event = pool_poll(&pool);
        if (event.task_id == POOL_ACCEPT)
        {
            pool_execute(&pool, accept_client, &server, POOL_ACCEPT);
            pool_execute(&pool, handle_client, event.ret, -1);
        }
        else
        {
            pool_execute(&pool, get_line, NULL, POOL_READLINE);
            printf("shell>>> %s\n", (char *)event.ret);
        }
    }
    pool_destroy(&pool);
    printf("ended:(\n");
    return 0;
}