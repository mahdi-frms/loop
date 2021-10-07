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
    char *rev = reverse(message);
    printf("client: %s", message);
    free(message);
    evloop_do_write_client(loop, client, rev);
}

void on_client(evloop_t *loop, int client)
{
    printf("new client\n");
    evloop_do_read_client(loop, client, on_message);
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

    evloop_t loop;
    evloop_initialize(&loop, threads);
    evloop_do_readline(&loop, on_line);
    evloop_do_accpet_client(&loop, &server, on_client);
    evloop_main_loop(&loop);
    evloop_destroy(&loop);
}