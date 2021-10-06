#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pool.h>
#include <server.h>

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

void *handle_client(void *arg)
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
    return NULL;
}

void *accept_client(void *arg)
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
    return client;
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
    pool_execute(&pool, accept_client, &server);
    while (1)
    {
        event_t event = pool_poll(&pool);
        pool_execute(&pool, accept_client, &server);
        pool_execute(&pool, handle_client, event);
    }
    pool_destroy(&pool);
    printf("ended:(\n");
    return 0;
}